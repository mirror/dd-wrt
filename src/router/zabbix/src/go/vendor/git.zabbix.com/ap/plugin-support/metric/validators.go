/*
** Zabbix
** Copyright 2001-2022 Zabbix SIA
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions
** of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

// Package metric provides an interface for describing a schema of metric's parameters.
package metric

import (
	"fmt"
	"regexp"
	"strconv"
	"strings"
)

type Validator interface {
	Validate(value *string) error
}

type SetValidator struct {
	Set             []string
	CaseInsensitive bool
}

func (v SetValidator) Validate(value *string) error {
	if v.Set != nil && len(v.Set) == 0 {
		panic("set cannot be empty")
	}

	if value == nil {
		return nil
	}

	for _, s := range v.Set {
		if (v.CaseInsensitive && strings.ToLower(*value) == strings.ToLower(s)) || (!v.CaseInsensitive && *value == s) {
			return nil
		}
	}

	return fmt.Errorf("allowed values: %s", strings.Join(v.Set, ", "))
}

type PatternValidator struct {
	Pattern string
}

func (v PatternValidator) Validate(value *string) error {
	if value == nil {
		return nil
	}

	b, err := regexp.MatchString(v.Pattern, *value)
	if err != nil {
		return err
	}

	if !b {
		return fmt.Errorf("value does not match pattern %q", v.Pattern)
	}

	return nil
}

type LenValidator struct {
	Min *int
	Max *int
}

func (v LenValidator) Validate(value *string) error {
	if value == nil || (v.Min == nil && v.Max == nil) {
		return nil
	}

	if v.Min != nil && len(*value) < *v.Min {
		return fmt.Errorf("value cannot be shorter than %d characters", v.Min)
	}

	if v.Max != nil && len(*value) > *v.Max {
		return fmt.Errorf("value cannot be longer than %d characters", v.Max)
	}

	return nil
}

type RangeValidator struct {
	Min int
	Max int
}

func (v RangeValidator) Validate(value *string) error {
	if value == nil {
		return nil
	}

	intVal, err := strconv.Atoi(*value)
	if err != nil {
		return err
	}

	if intVal < v.Min || intVal > v.Max {
		return fmt.Errorf("value is out of range [%d..%d]", v.Min, v.Max)
	}

	return nil
}

type NumberValidator struct{}

func (v NumberValidator) Validate(value *string) error {
	if value == nil {
		return nil
	}

	_, err := strconv.Atoi(*value)

	return err
}
