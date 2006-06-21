/*
 * ndis_events - Receive NdisMIndicateStatus() events using WMI
 * Copyright (c) 2004-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#define _WIN32_WINNT    0x0400

#include "includes.h"

#ifndef COBJMACROS
#define COBJMACROS
#endif /* COBJMACROS */
#include <wbemidl.h>

#include "common.h"


struct ndis_events_data {
	IWbemObjectSink sink;
	IWbemObjectSinkVtbl sink_vtbl;

	IWbemServices *pSvc;
	IWbemLocator *pLoc;

	HANDLE read_pipe, write_pipe, event_avail;
	UINT ref;
	int terminating;
};

enum event_types { EVENT_CONNECT, EVENT_DISCONNECT, EVENT_MEDIA_SPECIFIC,
		   EVENT_ADAPTER_ARRIVAL, EVENT_ADAPTER_REMOVAL };


static int ndis_events_constructor(struct ndis_events_data *events)
{
	events->ref = 1;

	if (!CreatePipe(&events->read_pipe, &events->write_pipe, NULL, 512)) {
		wpa_printf(MSG_ERROR, "CreatePipe() failed: %d",
			   (int) GetLastError());
		return -1;
	}
	events->event_avail = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (events->event_avail == NULL) {
		wpa_printf(MSG_ERROR, "CreateEvent() failed: %d",
			   (int) GetLastError());
		CloseHandle(events->read_pipe);
		CloseHandle(events->write_pipe);
		return -1;
	}

	return 0;
}


static void ndis_events_destructor(struct ndis_events_data *events)
{
	CloseHandle(events->read_pipe);
	CloseHandle(events->write_pipe);
	CloseHandle(events->event_avail);
	IWbemServices_Release(events->pSvc);
	IWbemLocator_Release(events->pLoc);
	CoUninitialize();
}


static HRESULT STDMETHODCALLTYPE
ndis_events_query_interface(IWbemObjectSink *this, REFIID riid, void **obj)
{
	*obj = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
	    IsEqualIID(riid, &IID_IWbemObjectSink)) {
		*obj = this;
		IWbemObjectSink_AddRef(this);
		return NOERROR;
	}

	return E_NOINTERFACE;
}


static ULONG STDMETHODCALLTYPE ndis_events_add_ref(IWbemObjectSink *this)
{
	struct ndis_events_data *events = (struct ndis_events_data *) this;
	return ++events->ref;
}


static ULONG STDMETHODCALLTYPE ndis_events_release(IWbemObjectSink *this)
{
	struct ndis_events_data *events = (struct ndis_events_data *) this;

	if (--events->ref != 0)
		return events->ref;

	ndis_events_destructor(events);
	wpa_printf(MSG_DEBUG, "ndis_events: terminated");
	free(events);
	return 0;
}


static int ndis_events_send_event(struct ndis_events_data *events,
				  enum event_types type, BSTR instance,
				  char *data, size_t data_len)
{
	char buf[512], *pos, *end;
	int len, _type;
	DWORD written;

	end = buf + sizeof(buf);
	_type = (int) type;
	memcpy(buf, &_type, sizeof(_type));
	pos = buf + sizeof(_type);

	len = _snprintf(pos + 1, end - pos - 1, "%S", instance);
	if (len < 0)
		return -1;
	if (len > 255)
		len = 255;
	*pos = (unsigned char) len;
	pos += 1 + len;
	if (data) {
		if (data_len > 255 || 1 + data_len > (size_t) (end - pos)) {
			wpa_printf(MSG_DEBUG, "Not enough room for send_event "
				   "data (%d)", data_len);
			return -1;
		}
		*pos++ = (unsigned char) data_len;
		memcpy(pos, data, data_len);
		pos += data_len;
	}

	if (WriteFile(events->write_pipe, buf, pos - buf, &written, NULL)) {
		SetEvent(events->event_avail);
		return 0;
	}
	wpa_printf(MSG_INFO, "WriteFile() failed: %d", (int) GetLastError());
	return -1;
}


static void ndis_events_media_connect(struct ndis_events_data *events,
				      IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	wpa_printf(MSG_DEBUG, "MSNdis_StatusMediaConnect");
	hr = IWbemClassObject_Get(pObj, L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		wpa_printf(MSG_DEBUG, "  InstanceName: '%S'", vt.bstrVal);
		ndis_events_send_event(events, EVENT_CONNECT, vt.bstrVal,
				       NULL, 0);
		VariantClear(&vt);
	}
}


static void ndis_events_media_disconnect(struct ndis_events_data *events,
					 IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	wpa_printf(MSG_DEBUG, "MSNdis_StatusMediaDisconnect");
	hr = IWbemClassObject_Get(pObj, L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		wpa_printf(MSG_DEBUG, "  InstanceName: '%S'", vt.bstrVal);
		ndis_events_send_event(events, EVENT_DISCONNECT, vt.bstrVal,
				       NULL, 0);
		VariantClear(&vt);
	}
}


static void ndis_events_media_specific(struct ndis_events_data *events,
				       IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	LONG lower, upper, k;
	UCHAR ch;
	char *data, *pos;
	size_t data_len;

	wpa_printf(MSG_DEBUG, "MSNdis_StatusMediaSpecificIndication");

	/* This is the StatusBuffer from NdisMIndicateStatus() call */
	hr = IWbemClassObject_Get(pObj, L"NdisStatusMediaSpecificIndication",
				  0, &vt, NULL, NULL);
	if (FAILED(hr)) {
		wpa_printf(MSG_DEBUG, "Could not get "
			   "NdisStatusMediaSpecificIndication from "
			   "the object?!");
		return;
	}

	SafeArrayGetLBound(V_ARRAY(&vt), 1, &lower);
	SafeArrayGetUBound(V_ARRAY(&vt), 1, &upper);
	data_len = upper - lower + 1;
	data = malloc(data_len);
	if (data == NULL) {
		wpa_printf(MSG_DEBUG, "Failed to allocate buffer for event "
			   "data");
		VariantClear(&vt);
		return;
	}

	pos = data;
	for (k = lower; k <= upper; k++) {
		SafeArrayGetElement(V_ARRAY(&vt), &k, &ch);
		*pos++ = ch;
	}
	wpa_hexdump(MSG_DEBUG, "MediaSpecificEvent", data, data_len);

	VariantClear(&vt);

	hr = IWbemClassObject_Get(pObj, L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		wpa_printf(MSG_DEBUG, "  InstanceName: '%S'", vt.bstrVal);
		ndis_events_send_event(events, EVENT_MEDIA_SPECIFIC,
				       vt.bstrVal, data, data_len);
		VariantClear(&vt);
	}

	free(data);
}


static void ndis_events_adapter_arrival(struct ndis_events_data *events,
					IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	wpa_printf(MSG_DEBUG, "MSNdis_NotifierAdapterArrival");
	hr = IWbemClassObject_Get(pObj, L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		wpa_printf(MSG_DEBUG, "  InstanceName: '%S'", vt.bstrVal);
		ndis_events_send_event(events, EVENT_ADAPTER_ARRIVAL,
				       vt.bstrVal, NULL, 0);
		VariantClear(&vt);
	}
}


static void ndis_events_adapter_removal(struct ndis_events_data *events,
					IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	wpa_printf(MSG_DEBUG, "MSNdis_NotifierAdapterRemoval");
	hr = IWbemClassObject_Get(pObj, L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		wpa_printf(MSG_DEBUG, "  InstanceName: '%S'", vt.bstrVal);
		ndis_events_send_event(events, EVENT_ADAPTER_REMOVAL,
				       vt.bstrVal, NULL, 0);
		VariantClear(&vt);
	}
}


static HRESULT STDMETHODCALLTYPE
ndis_events_indicate(IWbemObjectSink *this, long lObjectCount,
		     IWbemClassObject __RPC_FAR *__RPC_FAR *ppObjArray)
{
	struct ndis_events_data *events = (struct ndis_events_data *) this;
	long i;

	if (events->terminating) {
		wpa_printf(MSG_DEBUG, "ndis_events_indicate: Ignore "
			   "indication - terminating");
		return WBEM_NO_ERROR;
	}
	/* wpa_printf(MSG_DEBUG, "Notification received - %d object(s)",
	   lObjectCount); */

	for (i = 0; i < lObjectCount; i++) {
		IWbemClassObject *pObj = ppObjArray[i];
		HRESULT hr;
		VARIANT vtClass;

		hr = IWbemClassObject_Get(pObj, L"__CLASS", 0, &vtClass, NULL,
					  NULL);
		if (FAILED(hr)) {
			wpa_printf(MSG_DEBUG, "Failed to get __CLASS from "
				   "event.");
			break;
		}
		/* wpa_printf(MSG_DEBUG, "CLASS: '%S'", vtClass.bstrVal); */

		if (wcscmp(vtClass.bstrVal,
			   L"MSNdis_StatusMediaSpecificIndication") == 0) {
			ndis_events_media_specific(events, pObj);
		} else if (wcscmp(vtClass.bstrVal,
				  L"MSNdis_StatusMediaConnect") == 0) {
			ndis_events_media_connect(events, pObj);
		} else if (wcscmp(vtClass.bstrVal,
				  L"MSNdis_StatusMediaDisconnect") == 0) {
			ndis_events_media_disconnect(events, pObj);
		} else if (wcscmp(vtClass.bstrVal,
				  L"MSNdis_NotifyAdapterArrival") == 0) {
			ndis_events_adapter_arrival(events, pObj);
		} else if (wcscmp(vtClass.bstrVal,
				  L"MSNdis_NotifyAdapterRemoval") == 0) {
			ndis_events_adapter_removal(events, pObj);
		} else {
			wpa_printf(MSG_DEBUG, "Unepected event - __CLASS: "
				   "'%S'", vtClass.bstrVal);
		}

		VariantClear(&vtClass);
	}

	return WBEM_NO_ERROR;
}


static HRESULT STDMETHODCALLTYPE
ndis_events_set_status(IWbemObjectSink *this, long lFlags, HRESULT hResult,
		       BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam)
{
	return WBEM_NO_ERROR;
}


static int register_async_notification(IWbemObjectSink *pDestSink,
				       IWbemServices *pSvc)
{
	HRESULT hr;
	int err = 0;
	BSTR lang = SysAllocString(L"WQL");

	BSTR query = SysAllocString(
		L"SELECT * FROM MSNdis_StatusMediaConnect");
	hr = IWbemServices_ExecNotificationQueryAsync(pSvc, lang, query, 0, 0,
						      pDestSink);
	SysFreeString(query);
	if (FAILED(hr)) {
		wpa_printf(MSG_DEBUG, "ExecNotificationQueryAsync for "
			   "MSNdis_StatusMediaConnect failed with hresult of "
			   "0x%x", (int) hr);
		err = -1;
	}

	query = SysAllocString(
		L"SELECT * FROM MSNdis_StatusMediaDisconnect");
	hr = IWbemServices_ExecNotificationQueryAsync(pSvc, lang, query, 0, 0,
						      pDestSink);
	SysFreeString(query);
	if (FAILED(hr)) {
		wpa_printf(MSG_DEBUG, "ExecNotificationQueryAsync for "
			   "MSNdis_StatusMediaDisconnect failed with hresult "
			   "of 0x%x", (int) hr);
		err = -1;
	}

	query = SysAllocString(
		L"SELECT * FROM MSNdis_StatusMediaSpecificIndication");
	hr = IWbemServices_ExecNotificationQueryAsync(pSvc, lang, query, 0, 0,
						      pDestSink);
	SysFreeString(query);
	if (FAILED(hr)) {
		wpa_printf(MSG_DEBUG, "ExecNotificationQueryAsync for "
			   "MSNdis_StatusMediaSpecificIndication failed with "
			   "hresult of 0x%x", (int) hr);
		err = -1;
	}

	query = SysAllocString(
		L"SELECT * FROM MSNdis_NotifyAdapterArrival");
	hr = IWbemServices_ExecNotificationQueryAsync(pSvc, lang, query, 0, 0,
						      pDestSink);
	SysFreeString(query);
	if (FAILED(hr)) {
		wpa_printf(MSG_DEBUG, "ExecNotificationQueryAsync for "
			   "MSNdis_NotifyAdapterArrival failed with hresult "
			   "of 0x%x", (int) hr);
		err = -1;
	}

	query = SysAllocString(
		L"SELECT * FROM MSNdis_NotifyAdapterRemoval");
	hr = IWbemServices_ExecNotificationQueryAsync(pSvc, lang, query, 0, 0,
						      pDestSink);
	SysFreeString(query);
	if (FAILED(hr)) {
		wpa_printf(MSG_DEBUG, "ExecNotificationQueryAsync for "
			   "MSNdis_NotifyAdapterRemoval failed with hresult "
			   "of 0x%x", (int) hr);
		err = -1;
	}

	SysFreeString(lang);

	return err;
}


void ndis_events_deinit(struct ndis_events_data *events)
{
	events->terminating = 1;
	IWbemServices_CancelAsyncCall(events->pSvc, &events->sink);
	IWbemObjectSink_Release(&events->sink);
	/*
	 * Rest of deinitialization is done in ndis_events_destructor() once
	 * all reference count drops to zero.
	 */
}


struct ndis_events_data *
ndis_events_init(HANDLE *read_pipe, HANDLE *event_avail)
{
	HRESULT hr;
	IWbemObjectSink *pSink;
	struct ndis_events_data *events;

	events = wpa_zalloc(sizeof(*events));
	if (events == NULL) {
		wpa_printf(MSG_ERROR, "Could not allocate sink for events.");
		return NULL;
	}

	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		wpa_printf(MSG_ERROR, "CoInitializeEx() failed - returned "
			   "0x%x", (int) hr);
		free(events);
		return NULL;
	}

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
				  RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
				  RPC_C_IMP_LEVEL_IMPERSONATE,
				  NULL, EOAC_SECURE_REFS, NULL);
	if (FAILED(hr)) {
		wpa_printf(MSG_ERROR, "CoInitializeSecurity() failed - "
			   "returned 0x%x", (int) hr);
		free(events);
		return NULL;
	}

	hr = CoCreateInstance(&CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
			      &IID_IWbemLocator, (LPVOID *) &events->pLoc);
	if (FAILED(hr)) {
		wpa_printf(MSG_ERROR, "CoCreateInstance() failed - returned "
			   "0x%x", (int) hr);
		CoUninitialize();
		free(events);
		return NULL;
	}

	hr = IWbemLocator_ConnectServer(events->pLoc, L"ROOT\\WMI", NULL, NULL,
					0, 0, 0, 0, &events->pSvc);
	if (hr) {
		wpa_printf(MSG_ERROR, "Could not connect to server - error "
			   "0x%x", (int) hr);
		CoUninitialize();
		free(events);
		return NULL;
	}
	wpa_printf(MSG_DEBUG, "Connected to ROOT\\WMI.");

	ndis_events_constructor(events);
	pSink = &events->sink;
	pSink->lpVtbl = &events->sink_vtbl;
	events->sink_vtbl.QueryInterface = ndis_events_query_interface;
	events->sink_vtbl.AddRef = ndis_events_add_ref;
	events->sink_vtbl.Release = ndis_events_release;
	events->sink_vtbl.Indicate = ndis_events_indicate;
	events->sink_vtbl.SetStatus = ndis_events_set_status;

	if (register_async_notification(pSink, events->pSvc) < 0) {
		wpa_printf(MSG_DEBUG, "Failed to register async "
			   "notifications");
		CoUninitialize();
		ndis_events_destructor(events);
		free(events);
		return NULL;
	}

	*read_pipe = events->read_pipe;
	*event_avail = events->event_avail;

	return events;
}
