// Copyright 2025 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#include <stdlib.h>
#include "dpiImpl.h"

// C Wrapper Function to Set the Dimensions Union Field
void setVectorInfoDimensions(dpiVectorInfo *info, void *ptr) {
    info->dimensions.asPtr = ptr;
}

// C Wrapper Function to Get the Dimensions Union Field
void* getVectorInfoDimensions(dpiVectorInfo *info) {
    return info->dimensions.asPtr;
}

*/
import "C"

import (
	"fmt"
	"unsafe"
)

// Vector holds the embedding VECTOR column starting from 23ai.
type Vector struct {
	Dimensions uint32      // Total dimensions of the vector.
	Indices    []uint32    // Indices of non-zero values (sparse format).
	Values     interface{} // Non-zero values (sparse format) or all values (dense format).
	IsSparse   bool        // Flag to detect if it's a sparse vector
}

// SetVectorValue converts a Go `Vector` into a godror data type.
func SetVectorValue(c *conn, v *Vector, data *C.dpiData) error {
	var sparseIndices *C.uint32_t = nil
	numSparseValues := len(v.Indices)

	var vectorInfo C.dpiVectorInfo
	var valuesPtr unsafe.Pointer
	var format C.uint8_t
	var dimensionSize uint8
	var numDims int

	switch values := (*v).Values.(type) {
	case []float32:
		numDims, format, dimensionSize = len(values), C.DPI_VECTOR_FORMAT_FLOAT32, 4
		if numDims > 0 {
			valuesPtr = unsafe.Pointer(&values[0])
		}
	case []float64:
		numDims, format, dimensionSize = len(values), C.DPI_VECTOR_FORMAT_FLOAT64, 8
		if numDims > 0 {
			valuesPtr = unsafe.Pointer(&values[0])
		}
	case []int8:
		numDims, format, dimensionSize = len(values), C.DPI_VECTOR_FORMAT_INT8, 1
		if numDims > 0 {
			valuesPtr = unsafe.Pointer(&values[0])
		}
	case []uint8:
		numDims, format, dimensionSize = len(values), C.DPI_VECTOR_FORMAT_BINARY, 1
		if numDims > 0 {
			valuesPtr = unsafe.Pointer(&values[0])
		}
	default:
		return fmt.Errorf("SetVectorValue Unsupported type: %T in Vector Values", v.Values)
	}
	C.setVectorInfoDimensions(&vectorInfo, valuesPtr) //update values

	// update sparse indices and numDimensions
	if v.IsSparse || numSparseValues > 0 {
		if numSparseValues > 0 {
			sparseIndices = (*C.uint32_t)(C.malloc(C.size_t(numSparseValues) * C.size_t(unsafe.Sizeof(C.uint32_t(0)))))
			defer C.free(unsafe.Pointer(sparseIndices))
			cArray := unsafe.Slice((*C.uint32_t)(unsafe.Pointer(sparseIndices)), numSparseValues)
			for i, val := range v.Indices {
				cArray[i] = C.uint32_t(val)
			}
			// below causes hang for uint32 alone ..
			//sparseIndices = (*C.uint32_t)(unsafe.Pointer(&(v.Indices[0])))
		}
		vectorInfo.numDimensions = C.uint32_t(v.Dimensions)
	} else {
		// update numDimensions for Dense
		if format == C.DPI_VECTOR_FORMAT_BINARY {
			numDims *= 8 // Each byte represents 8 dimensions.
		}
		vectorInfo.numDimensions = C.uint32_t(numDims)
	}

	// Populate vectorInfo.
	vectorInfo.format = format
	vectorInfo.dimensionSize = C.uint8_t(dimensionSize)
	vectorInfo.numSparseValues = C.uint32_t(numSparseValues)
	vectorInfo.sparseIndices = (*C.uint32_t)(sparseIndices)

	// Set vector value.
	if err := c.checkExec(func() C.int {
		return C.dpiVector_setValue(C.dpiData_getVector(data), &vectorInfo)
	}); err != nil {
		return fmt.Errorf("SetVectorValue %w", err)
	}
	return nil
}

// GetVectorValue converts a C `dpiVectorInfo` struct into a Go `Vector`
func GetVectorValue(vectorInfo *C.dpiVectorInfo) (Vector, error) {
	var values interface{}
	var indices []uint32
	var isSparse bool

	var nonZeroVal = vectorInfo.numDimensions

	// create Indices
	if vectorInfo.numSparseValues > 0 {
		isSparse = true
		nonZeroVal = vectorInfo.numSparseValues
		indices = make([]uint32, vectorInfo.numSparseValues)
		ptr := unsafe.Slice((*C.uint32_t)(unsafe.Pointer(vectorInfo.sparseIndices)), int(vectorInfo.numSparseValues))
		for i, v := range ptr {
			indices[i] = uint32(v)
		}
	}

	// create Values
	switch vectorInfo.format {
	case C.DPI_VECTOR_FORMAT_FLOAT32:
		ptr := unsafe.Slice((*float32)(unsafe.Pointer(C.getVectorInfoDimensions(vectorInfo))), int(vectorInfo.numDimensions))
		values = make([]float32, nonZeroVal)
		copy(values.([]float32), ptr)
	case C.DPI_VECTOR_FORMAT_FLOAT64:
		ptr := unsafe.Slice((*float64)(unsafe.Pointer(C.getVectorInfoDimensions(vectorInfo))), int(vectorInfo.numDimensions))
		values = make([]float64, nonZeroVal)
		copy(values.([]float64), ptr)
	case C.DPI_VECTOR_FORMAT_INT8:
		ptr := unsafe.Slice((*int8)(unsafe.Pointer(C.getVectorInfoDimensions(vectorInfo))), int(vectorInfo.numDimensions))
		values = make([]int8, nonZeroVal)
		copy(values.([]int8), ptr)
	case C.DPI_VECTOR_FORMAT_BINARY:
		size := vectorInfo.numDimensions / 8
		ptr := unsafe.Slice((*uint8)(unsafe.Pointer(C.getVectorInfoDimensions(vectorInfo))), int(vectorInfo.numDimensions))
		values = make([]uint8, size)
		copy(values.([]uint8), ptr)
	default:
		return Vector{}, fmt.Errorf("GetVectorValue Unknown VECTOR format: %d", vectorInfo.format)
	}

	return Vector{
		Indices:    indices,
		Dimensions: uint32(vectorInfo.numDimensions),
		Values:     values,
		IsSparse:   isSparse,
	}, nil
}
