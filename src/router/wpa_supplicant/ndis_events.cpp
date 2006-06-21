/*
 * ndis_events - test program for receiving NdisMIndicateStatus() events
 * Copyright (c) 2004-2005, Jouni Malinen <jkmaline@cc.hut.fi>
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

#include <windows.h>
#include <stdio.h>
#include <wbemidl.h>
#include <winsock.h>

struct ndis_events_data {
};


class CNdisSink : public IWbemObjectSink
{
public:
	CNdisSink();
	~CNdisSink();

	// IUnknown members
	STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	virtual HRESULT STDMETHODCALLTYPE Indicate(
		long lObjectCount,
		IWbemClassObject __RPC_FAR *__RPC_FAR *ppObjArray);

	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		long lFlags, HRESULT hResult, BSTR strParam,
		IWbemClassObject __RPC_FAR *pObjParam);

#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
	HANDLE read_pipe, event_avail;
#endif /* CONFIG_NDIS_EVENTS_INTEGRATED */

private:
	void media_connect(IWbemClassObject *pObj);
	void media_disconnect(IWbemClassObject *pObj);
	void media_specific(IWbemClassObject *pObj);
	enum event_types { EVENT_CONNECT, EVENT_DISCONNECT,
			   EVENT_MEDIA_SPECIFIC };
	int send_event(enum event_types type, BSTR instance,
		       char *data = NULL, size_t data_len = 0);

	UINT m_cRef;

#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
	HANDLE write_pipe;
#else /* CONFIG_NDIS_EVENTS_INTEGRATED */
	SOCKET sock;
	struct sockaddr_in dst;
#endif /* CONFIG_NDIS_EVENTS_INTEGRATED */
};


CNdisSink::CNdisSink()
{
	m_cRef = 1;

#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
	if (!CreatePipe(&read_pipe, &write_pipe, NULL, 512)) {
		printf("CreatePipe() failed: %d\n", (int) GetLastError());
		return;
	}
	event_avail = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (event_avail == NULL) {
		printf("CreateEvent() failed: %d\n", (int) GetLastError());
		CloseHandle(read_pipe);
		CloseHandle(write_pipe);
		return;
	}
#else /* CONFIG_NDIS_EVENTS_INTEGRATED */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		perror("socket");
		return;
	}

	memset(&dst, 0, sizeof(dst));
	dst.sin_family = AF_INET;
	dst.sin_addr.s_addr = inet_addr("127.0.0.1");
	dst.sin_port = htons(9876);
#endif /* CONFIG_NDIS_EVENTS_INTEGRATED */
}


CNdisSink::~CNdisSink()
{
#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
	CloseHandle(read_pipe);
	CloseHandle(write_pipe);
	CloseHandle(event_avail);
#else /* CONFIG_NDIS_EVENTS_INTEGRATED */
	if (sock != INVALID_SOCKET)
		closesocket(sock);
#endif /* CONFIG_NDIS_EVENTS_INTEGRATED */
}


STDMETHODIMP CNdisSink::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = 0;

	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}


ULONG CNdisSink::AddRef()
{
	return ++m_cRef;
}


ULONG CNdisSink::Release()
{
	if (--m_cRef != 0)
		return m_cRef;

	delete this;
	return 0;
}


int CNdisSink::send_event(enum event_types type, BSTR instance,
			  char *data, size_t data_len)
{
	char buf[512], *pos, *end;
	int len, _type;

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
			printf("Not enough room for send_event data (%d)\n",
			       data_len);
			return -1;
		}
		*pos++ = (unsigned char) data_len;
		memcpy(pos, data, data_len);
		pos += data_len;
	}

#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
	DWORD written;
	if (WriteFile(write_pipe, buf, pos - buf, &written, NULL)) {
		SetEvent(event_avail);
		return 0;
	}
	printf("WriteFile() failed: %d\n", (int) GetLastError());
	return -1;
#else /* CONFIG_NDIS_EVENTS_INTEGRATED */
	return sendto(sock, buf, pos - buf, 0, (struct sockaddr *) &dst,
		      sizeof(dst));
#endif /* CONFIG_NDIS_EVENTS_INTEGRATED */
}


void CNdisSink::media_connect(IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	printf("MSNdis_StatusMediaConnect\n");
	hr = pObj->Get(L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		printf("  InstanceName: '%S'\n", vt.bstrVal);
		send_event(EVENT_CONNECT, vt.bstrVal);
		VariantClear(&vt);
	}
}


void CNdisSink::media_disconnect(IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	printf("MSNdis_StatusMediaDisconnect\n");
	hr = pObj->Get(L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		printf("  InstanceName: '%S'\n", vt.bstrVal);
		send_event(EVENT_DISCONNECT, vt.bstrVal);
		VariantClear(&vt);
	}
}


void CNdisSink::media_specific(IWbemClassObject *pObj)
{
	VARIANT vt;
	HRESULT hr;
	LONG lower, upper, k;
	UCHAR ch;
	char *data, *pos;
	size_t data_len;

	printf("MSNdis_StatusMediaSpecificIndication\n");

	/* This is the StatusBuffer from NdisMIndicateStatus() call */
	hr = pObj->Get(L"NdisStatusMediaSpecificIndication", 0, &vt, NULL,
		       NULL);
	if (FAILED(hr)) {
		printf("Could not get NdisStatusMediaSpecificIndication from "
		       "the object?!\n");
		return;
	}

	SafeArrayGetLBound(V_ARRAY(&vt), 1, &lower);
	SafeArrayGetUBound(V_ARRAY(&vt), 1, &upper);
	data_len = upper - lower + 1;
	data = (char *) malloc(data_len);
	if (data == NULL) {
		printf("Failed to allocate buffer for event data\n");
		VariantClear(&vt);
		return;
	}

	printf("  Data(len=%d):", data_len);
	pos = data;
	for (k = lower; k <= upper; k++) {
		SafeArrayGetElement(V_ARRAY(&vt), &k, &ch);
		*pos++ = ch;
		printf(" %02x", ch);
	}
	printf("\n");

	VariantClear(&vt);

	hr = pObj->Get(L"InstanceName", 0, &vt, NULL, NULL);
	if (SUCCEEDED(hr)) {
		printf("  InstanceName: '%S'\n", vt.bstrVal);
		send_event(EVENT_MEDIA_SPECIFIC, vt.bstrVal, data, data_len);
		VariantClear(&vt);
	}

	free(data);
}


HRESULT CNdisSink::Indicate(long lObjectCount,
			    IWbemClassObject __RPC_FAR *__RPC_FAR *ppObjArray)
{
	//printf("Notification received - %d object(s)\n", lObjectCount);

	for (long i = 0; i < lObjectCount; i++) {
		IWbemClassObject *pObj = ppObjArray[i];
		HRESULT hr;
		VARIANT vtClass;

		hr = pObj->Get(L"__CLASS", 0, &vtClass, NULL, NULL);
		if (FAILED(hr)) {
			printf("Failed to get __CLASS from event.\n");
			break;
		}
		//printf("CLASS: '%S'\n", vtClass.bstrVal);

		if (wcscmp(vtClass.bstrVal,
			   L"MSNdis_StatusMediaSpecificIndication") == 0) {
			media_specific(pObj);
		} else if (wcscmp(vtClass.bstrVal,
				  L"MSNdis_StatusMediaConnect") == 0) {
			media_connect(pObj);
		} else if (wcscmp(vtClass.bstrVal,
				  L"MSNdis_StatusMediaDisconnect") == 0) {
			media_disconnect(pObj);
		} else {
			printf("Unepected event - __CLASS: '%S'\n",
			       vtClass.bstrVal);
		}

		VariantClear(&vtClass);
	}

	return WBEM_NO_ERROR;
}


HRESULT CNdisSink::SetStatus(long lFlags, HRESULT hResult, BSTR strParam,
			   IWbemClassObject __RPC_FAR *pObjParam)
{
	return WBEM_NO_ERROR;
}


static int register_async_notification(IWbemObjectSink *pDestSink,
				       IWbemServices *pSvc)
{
	HRESULT hr;
	int err = 0;
	BSTR lang = ::SysAllocString(L"WQL");

	BSTR query = ::SysAllocString(
		L"SELECT * FROM MSNdis_StatusMediaConnect");
	hr = pSvc->ExecNotificationQueryAsync(lang, query, 0, 0, pDestSink);
	::SysFreeString(query);
	if (FAILED(hr)) {
		printf("ExecNotificationQueryAsync for "
		       "MSNdis_StatusMediaConnect failed with hresult of "
		       "0x%x\n", hr);
		err = -1;
	}

	query = ::SysAllocString(
		L"SELECT * FROM MSNdis_StatusMediaDisconnect");
	hr = pSvc->ExecNotificationQueryAsync(lang, query, 0, 0, pDestSink);
	::SysFreeString(query);
	if (FAILED(hr)) {
		printf("ExecNotificationQueryAsync for "
		       "MSNdis_StatusMediaDisconnect failed with hresult of "
		       "0x%x\n", hr);
		err = -1;
	}

	query = ::SysAllocString(
		L"SELECT * FROM MSNdis_StatusMediaSpecificIndication");
	hr = pSvc->ExecNotificationQueryAsync(lang, query, 0, 0, pDestSink);
	::SysFreeString(query);
	if (FAILED(hr)) {
		printf("ExecNotificationQueryAsync for "
		       "MSNdis_StatusMediaSpecificIndication failed with "
		       "hresult of 0x%x\n", hr);
		err = -1;
	}

	::SysFreeString(lang);

	return err;
}


#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
extern "C" {
#endif

static IWbemServices *pSvc = 0;
static IWbemLocator *pLoc = NULL;
static CNdisSink *pSink = NULL;

void ndis_events_deinit(struct ndis_events_data *events)
{
	pSvc->CancelAsyncCall(pSink);
	pSink->Release();
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();
}


#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
int ndis_events_main(HANDLE *read_pipe, HANDLE *event_avail)
#else
int main(int argc, char *argv[])
#endif
{
	HRESULT hr;

	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		printf("CoInitializeEx() failed - returned 0x%x", hr);
		return -1;
	}

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
				  RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
				  RPC_C_IMP_LEVEL_IMPERSONATE,
				  NULL, EOAC_SECURE_REFS, NULL);
	if (FAILED(hr)) {
		printf("CoInitializeSecurity() failed - returned 0x%x", hr);
		return -1;
	}

	hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
			      IID_IWbemLocator, (LPVOID *) &pLoc);
	if (FAILED(hr)) {
		printf("CoCreateInstance() failed - returned 0x%x\n", hr);
		CoUninitialize();
		return -1;
	}

	hr = pLoc->ConnectServer(L"ROOT\\WMI", NULL, NULL, 0, 0, 0, 0, &pSvc);
	if (hr) {
		printf("Could not connect to server - error 0x%x\n", hr);
		CoUninitialize();
		return -1;
	}
	printf("Connected to ROOT\\WMI.\n");

#ifndef CONFIG_NDIS_EVENTS_INTEGRATED
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		printf("Could not find a usable winsock.dll.\n");
		CoUninitialize();
		return -1;
	}
#endif

	pSink = new CNdisSink;
	if (pSink == NULL) {
		printf("Could not allocate sink for events.\n");
		CoUninitialize();
		return -1;
	}

	if (register_async_notification(pSink, pSvc) < 0) {
		printf("Failed to register async notifications\n");
		CoUninitialize();
		return -1;
	}

#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
	*read_pipe = pSink->read_pipe;
	*event_avail = pSink->event_avail;
#else /* CONFIG_NDIS_EVENTS_INTEGRATED */
	/* Just wait.. sink will be called with events.. */
	while (getchar() != '\n');

	ndis_events_deinit(NULL);

	WSACleanup();
#endif /* CONFIG_NDIS_EVENTS_INTEGRATED */

	return 0;
}

#ifdef CONFIG_NDIS_EVENTS_INTEGRATED
struct ndis_events_data *
ndis_events_init(HANDLE *read_pipe, HANDLE *event_avail)
{
	struct ndis_events_data *events;

	events = (struct ndis_events_data *) malloc(sizeof(*events));
	if (events == NULL)
		return NULL;

	if (ndis_events_main(read_pipe, event_avail)) {
		free(events);
		return NULL;
	}

	return events;
}

}
#endif
