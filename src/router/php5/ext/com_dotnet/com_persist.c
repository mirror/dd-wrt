/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong  <wez@thebrainroom.com>                          |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/* Infrastructure for working with persistent COM objects.
 * Implements: IStream* wrapper for PHP streams.
 * TODO: Magic __wakeup and __sleep handlers for serialization 
 * (can wait till 5.1) */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_com_dotnet.h"
#include "php_com_dotnet_internal.h"
#include "Zend/zend_exceptions.h"

/* {{{ expose php_stream as a COM IStream */

typedef struct {
	CONST_VTBL struct IStreamVtbl *lpVtbl;
	DWORD engine_thread;
	LONG refcount;
	php_stream *stream;
	int id;
} php_istream;

static int le_istream;
static void istream_destructor(php_istream *stm TSRMLS_DC);

static void istream_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_istream *stm = (php_istream *)rsrc->ptr;
	istream_destructor(stm TSRMLS_CC);
}

#define FETCH_STM()	\
	php_istream *stm = (php_istream*)This; \
	TSRMLS_FETCH(); \
	if (GetCurrentThreadId() != stm->engine_thread) \
		return RPC_E_WRONG_THREAD;
		
#define FETCH_STM_EX()	\
	php_istream *stm = (php_istream*)This;	\
	if (GetCurrentThreadId() != stm->engine_thread)	\
		return RPC_E_WRONG_THREAD;

static HRESULT STDMETHODCALLTYPE stm_queryinterface(
	IStream *This,
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void **ppvObject)
{
	FETCH_STM_EX();

	if (IsEqualGUID(&IID_IUnknown, riid) ||
			IsEqualGUID(&IID_IStream, riid)) {
		*ppvObject = This;
		InterlockedIncrement(&stm->refcount);
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE stm_addref(IStream *This)
{
	FETCH_STM_EX();

	return InterlockedIncrement(&stm->refcount);
}
        
static ULONG STDMETHODCALLTYPE stm_release(IStream *This)
{
	ULONG ret;
	FETCH_STM();

	ret = InterlockedDecrement(&stm->refcount);
	if (ret == 0) {
		/* destroy it */
		if (stm->id)
			zend_list_delete(stm->id);
	}
	return ret;
}

static HRESULT STDMETHODCALLTYPE stm_read(IStream *This, void *pv, ULONG cb, ULONG *pcbRead)
{
	int nread;
	FETCH_STM();

	nread = php_stream_read(stm->stream, pv, cb);

	if (pcbRead) {
		*pcbRead = nread > 0 ? nread : 0;
	}
	if (nread > 0) {
		return S_OK;
	}
	return S_FALSE;
}

static HRESULT STDMETHODCALLTYPE stm_write(IStream *This, void const *pv, ULONG cb, ULONG *pcbWritten)
{
	int nwrote;
	FETCH_STM();

	nwrote = php_stream_write(stm->stream, pv, cb);

	if (pcbWritten) {
		*pcbWritten = nwrote > 0 ? nwrote : 0;
	}
	if (nwrote > 0) {
		return S_OK;
	}
	return S_FALSE;
}

static HRESULT STDMETHODCALLTYPE stm_seek(IStream *This, LARGE_INTEGER dlibMove,
		DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	off_t offset;
	int whence;
	int ret;
	FETCH_STM();

	switch (dwOrigin) {
		case STREAM_SEEK_SET:	whence = SEEK_SET;	break;
		case STREAM_SEEK_CUR:	whence = SEEK_CUR;	break;
		case STREAM_SEEK_END:	whence = SEEK_END;	break;
		default:
			return STG_E_INVALIDFUNCTION;
	}
	
	if (dlibMove.HighPart) {
		/* we don't support 64-bit offsets */
		return STG_E_INVALIDFUNCTION;
	}
	
	offset = (off_t) dlibMove.QuadPart;

	ret = php_stream_seek(stm->stream, offset, whence);

	if (plibNewPosition) {
		plibNewPosition->QuadPart = (ULONGLONG)(ret >= 0 ? ret : 0);
	}

	return ret >= 0 ? S_OK : STG_E_INVALIDFUNCTION;
}

static HRESULT STDMETHODCALLTYPE stm_set_size(IStream *This, ULARGE_INTEGER libNewSize)
{
	FETCH_STM();

	if (libNewSize.HighPart) {
		return STG_E_INVALIDFUNCTION;
	}
	
	if (php_stream_truncate_supported(stm->stream)) {
		int ret = php_stream_truncate_set_size(stm->stream, (size_t)libNewSize.QuadPart);

		if (ret == 0) {
			return S_OK;
		}
	}

	return STG_E_INVALIDFUNCTION;
}

static HRESULT STDMETHODCALLTYPE stm_copy_to(IStream *This, IStream *pstm, ULARGE_INTEGER cb,
		ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	FETCH_STM_EX();

	return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE stm_commit(IStream *This, DWORD grfCommitFlags)
{
	FETCH_STM();

	php_stream_flush(stm->stream);

	return S_OK;
}

static HRESULT STDMETHODCALLTYPE stm_revert(IStream *This)
{
	/* NOP */
	return S_OK;
}

static HRESULT STDMETHODCALLTYPE stm_lock_region(IStream *This,
   	ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD lockType)
{
	return STG_E_INVALIDFUNCTION;
}

static HRESULT STDMETHODCALLTYPE stm_unlock_region(IStream *This,
		ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD lockType)
{
	return STG_E_INVALIDFUNCTION;
}

static HRESULT STDMETHODCALLTYPE stm_stat(IStream *This,
		STATSTG *pstatstg, DWORD grfStatFlag)
{
	return STG_E_INVALIDFUNCTION;
}

static HRESULT STDMETHODCALLTYPE stm_clone(IStream *This, IStream **ppstm)
{
	return STG_E_INVALIDFUNCTION;
}

static struct IStreamVtbl php_istream_vtbl = {
	stm_queryinterface,
	stm_addref,
	stm_release,
	stm_read,
	stm_write,
	stm_seek,
	stm_set_size,
	stm_copy_to,
	stm_commit,
	stm_revert,
	stm_lock_region,
	stm_unlock_region,
	stm_stat,
	stm_clone
};

static void istream_destructor(php_istream *stm TSRMLS_DC)
{
	if (stm->id) {
		int id = stm->id;
		stm->id = 0;
		zend_list_delete(id);
		return;
	}

	if (stm->refcount > 0) {
		CoDisconnectObject((IUnknown*)stm, 0);
	}

	zend_list_delete(stm->stream->rsrc_id);

	CoTaskMemFree(stm);
}
/* }}} */

PHP_COM_DOTNET_API IStream *php_com_wrapper_export_stream(php_stream *stream TSRMLS_DC)
{
	php_istream *stm = (php_istream*)CoTaskMemAlloc(sizeof(*stm));

	if (stm == NULL)
		return NULL;

	memset(stm, 0, sizeof(*stm));
	stm->engine_thread = GetCurrentThreadId();
	stm->lpVtbl = &php_istream_vtbl;
	stm->refcount = 1;
	stm->stream = stream;

	zend_list_addref(stream->rsrc_id);
	stm->id = zend_list_insert(stm, le_istream TSRMLS_CC);

	return (IStream*)stm;
}

#define CPH_ME(fname, arginfo)	PHP_ME(com_persist, fname, arginfo, ZEND_ACC_PUBLIC)
#define CPH_SME(fname, arginfo)	PHP_ME(com_persist, fname, arginfo, ZEND_ACC_ALLOW_STATIC|ZEND_ACC_PUBLIC)
#define CPH_METHOD(fname)		static PHP_METHOD(com_persist, fname)
	
#define CPH_FETCH()				php_com_persist_helper *helper = (php_com_persist_helper*)zend_object_store_get_object(getThis() TSRMLS_CC);

#define CPH_NO_OBJ()			if (helper->unk == NULL) { php_com_throw_exception(E_INVALIDARG, "No COM object is associated with this helper instance" TSRMLS_CC); return; }

typedef struct {
	zend_object			std;
	long codepage;
	IUnknown 			*unk;
	IPersistStream 		*ips;
	IPersistStreamInit	*ipsi;
	IPersistFile		*ipf;
} php_com_persist_helper;

static zend_object_handlers helper_handlers;
static zend_class_entry *helper_ce;

static inline HRESULT get_persist_stream(php_com_persist_helper *helper)
{
	if (!helper->ips && helper->unk) {
		return IUnknown_QueryInterface(helper->unk, &IID_IPersistStream, &helper->ips);
	}
	return helper->ips ? S_OK : E_NOTIMPL;
}

static inline HRESULT get_persist_stream_init(php_com_persist_helper *helper)
{
	if (!helper->ipsi && helper->unk) {
		return IUnknown_QueryInterface(helper->unk, &IID_IPersistStreamInit, &helper->ipsi);
	}
	return helper->ipsi ? S_OK : E_NOTIMPL;
}

static inline HRESULT get_persist_file(php_com_persist_helper *helper)
{
	if (!helper->ipf && helper->unk) {
		return IUnknown_QueryInterface(helper->unk, &IID_IPersistFile, &helper->ipf);
	}
	return helper->ipf ? S_OK : E_NOTIMPL;
}


/* {{{ proto string COMPersistHelper::GetCurFile()
   Determines the filename into which an object will be saved, or false if none is set, via IPersistFile::GetCurFile */
CPH_METHOD(GetCurFileName)
{
	HRESULT res;
	OLECHAR *olename = NULL;
	CPH_FETCH();

	CPH_NO_OBJ();
	
	res = get_persist_file(helper);
	if (helper->ipf) {
		res = IPersistFile_GetCurFile(helper->ipf, &olename);

		if (res == S_OK) {
			Z_TYPE_P(return_value) = IS_STRING;
			Z_STRVAL_P(return_value) = php_com_olestring_to_string(olename,
				   &Z_STRLEN_P(return_value), helper->codepage TSRMLS_CC);
			CoTaskMemFree(olename);
			return;
		} else if (res == S_FALSE) {
			CoTaskMemFree(olename);
			RETURN_FALSE;
		}
		php_com_throw_exception(res, NULL TSRMLS_CC);
	} else {
		php_com_throw_exception(res, NULL TSRMLS_CC);
	}
}
/* }}} */


/* {{{ proto bool COMPersistHelper::SaveToFile(string filename [, bool remember])
   Persist object data to file, via IPersistFile::Save */
CPH_METHOD(SaveToFile)
{
	HRESULT res;
	char *filename, *fullpath = NULL;
	int filename_len;
	zend_bool remember = TRUE;
	OLECHAR *olefilename = NULL;
	CPH_FETCH();
	
	CPH_NO_OBJ();

	res = get_persist_file(helper);
	if (helper->ipf) {
		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p!|b",
					&filename, &filename_len, &remember)) {
			php_com_throw_exception(E_INVALIDARG, "Invalid arguments" TSRMLS_CC);
			return;
		}

		if (filename) {
			fullpath = expand_filepath(filename, NULL TSRMLS_CC);
			if (!fullpath) {
				RETURN_FALSE;
			}
	
			if (php_check_open_basedir(fullpath TSRMLS_CC)) {
				efree(fullpath);
				RETURN_FALSE;
			}

			olefilename = php_com_string_to_olestring(filename, strlen(fullpath), helper->codepage TSRMLS_CC);
			efree(fullpath);
		}
		res = IPersistFile_Save(helper->ipf, olefilename, remember);
		if (SUCCEEDED(res)) {
			if (!olefilename) {
				res = IPersistFile_GetCurFile(helper->ipf, &olefilename);
				if (S_OK == res) {
					IPersistFile_SaveCompleted(helper->ipf, olefilename);
					CoTaskMemFree(olefilename);
					olefilename = NULL;
				}
			} else if (remember) {
				IPersistFile_SaveCompleted(helper->ipf, olefilename);
			}
		}
			
		if (olefilename) {
			efree(olefilename);
		}

		if (FAILED(res)) {
			php_com_throw_exception(res, NULL TSRMLS_CC);
		}

	} else {
		php_com_throw_exception(res, NULL TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto bool COMPersistHelper::LoadFromFile(string filename [, int flags])
   Load object data from file, via IPersistFile::Load */
CPH_METHOD(LoadFromFile)
{
	HRESULT res;
	char *filename, *fullpath;
	int filename_len;
	long flags = 0;
	OLECHAR *olefilename;
	CPH_FETCH();
	
	CPH_NO_OBJ();

	res = get_persist_file(helper);
	if (helper->ipf) {

		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p|l",
					&filename, &filename_len, &flags)) {
			php_com_throw_exception(E_INVALIDARG, "Invalid arguments" TSRMLS_CC);
			return;
		}

		if (!(fullpath = expand_filepath(filename, NULL TSRMLS_CC))) {
			RETURN_FALSE;
		}

		if (php_check_open_basedir(fullpath TSRMLS_CC)) {
			efree(fullpath);
			RETURN_FALSE;
		}

		olefilename = php_com_string_to_olestring(fullpath, strlen(fullpath), helper->codepage TSRMLS_CC);
		efree(fullpath);
			
		res = IPersistFile_Load(helper->ipf, olefilename, flags);
		efree(olefilename);

		if (FAILED(res)) {
			php_com_throw_exception(res, NULL TSRMLS_CC);
		}
		
	} else {
		php_com_throw_exception(res, NULL TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto int COMPersistHelper::GetMaxStreamSize()
   Gets maximum stream size required to store the object data, via IPersistStream::GetSizeMax (or IPersistStreamInit::GetSizeMax) */
CPH_METHOD(GetMaxStreamSize)
{
	HRESULT res;
	ULARGE_INTEGER size;
	CPH_FETCH();
	
	CPH_NO_OBJ();
	
	res = get_persist_stream_init(helper);
	if (helper->ipsi) {
		res = IPersistStreamInit_GetSizeMax(helper->ipsi, &size);
	} else {
		res = get_persist_stream(helper);
		if (helper->ips) {
			res = IPersistStream_GetSizeMax(helper->ips, &size);
		} else {
			php_com_throw_exception(res, NULL TSRMLS_CC);
			return;
		}
	}

	if (res != S_OK) {
		php_com_throw_exception(res, NULL TSRMLS_CC);
	} else {
		/* TODO: handle 64 bit properly */
		RETURN_LONG((LONG)size.QuadPart);
	}
}
/* }}} */

/* {{{ proto int COMPersistHelper::InitNew()
   Initializes the object to a default state, via IPersistStreamInit::InitNew */
CPH_METHOD(InitNew)
{
	HRESULT res;
	CPH_FETCH();
	
	CPH_NO_OBJ();

	res = get_persist_stream_init(helper);
	if (helper->ipsi) {
		res = IPersistStreamInit_InitNew(helper->ipsi);

		if (res != S_OK) {
			php_com_throw_exception(res, NULL TSRMLS_CC);
		} else {
			RETURN_TRUE;
		}
	} else {
		php_com_throw_exception(res, NULL TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto mixed COMPersistHelper::LoadFromStream(resource stream)
   Initializes an object from the stream where it was previously saved, via IPersistStream::Load or OleLoadFromStream */
CPH_METHOD(LoadFromStream)
{
	zval *zstm;
	php_stream *stream;
	IStream *stm = NULL;
	HRESULT res;
	CPH_FETCH();
	
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zstm)) {
		php_com_throw_exception(E_INVALIDARG, "invalid arguments" TSRMLS_CC);
		return;
	}

	php_stream_from_zval_no_verify(stream, &zstm);
	
	if (stream == NULL) {
		php_com_throw_exception(E_INVALIDARG, "expected a stream" TSRMLS_CC);
		return;
	}

	stm = php_com_wrapper_export_stream(stream TSRMLS_CC);
	if (stm == NULL) {
		php_com_throw_exception(E_UNEXPECTED, "failed to wrap stream" TSRMLS_CC);
		return;
	}
	
	res = S_OK;
	RETVAL_TRUE;

	if (helper->unk == NULL) {
		IDispatch *disp = NULL;

		/* we need to create an object and load using OleLoadFromStream */
		res = OleLoadFromStream(stm, &IID_IDispatch, &disp);

		if (SUCCEEDED(res)) {
			php_com_wrap_dispatch(return_value, disp, COMG(code_page) TSRMLS_CC);	
		}
	} else {
		res = get_persist_stream_init(helper);
		if (helper->ipsi) {
			res = IPersistStreamInit_Load(helper->ipsi, stm);
		} else {
			res = get_persist_stream(helper);
			if (helper->ips) {
				res = IPersistStreamInit_Load(helper->ipsi, stm);
			}
		}
	}
	IStream_Release(stm);

	if (FAILED(res)) {
		php_com_throw_exception(res, NULL TSRMLS_CC);
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto int COMPersistHelper::SaveToStream(resource stream)
   Saves the object to a stream, via IPersistStream::Save */
CPH_METHOD(SaveToStream)
{
	zval *zstm;
	php_stream *stream;
	IStream *stm = NULL;
	HRESULT res;
	CPH_FETCH();
	
	CPH_NO_OBJ();
	
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zstm)) {
		php_com_throw_exception(E_INVALIDARG, "invalid arguments" TSRMLS_CC);
		return;
	}

	php_stream_from_zval_no_verify(stream, &zstm);
	
	if (stream == NULL) {
		php_com_throw_exception(E_INVALIDARG, "expected a stream" TSRMLS_CC);
		return;
	}

	stm = php_com_wrapper_export_stream(stream TSRMLS_CC);
	if (stm == NULL) {
		php_com_throw_exception(E_UNEXPECTED, "failed to wrap stream" TSRMLS_CC);
		return;
	}
	
	res = get_persist_stream_init(helper);
	if (helper->ipsi) {
		res = IPersistStreamInit_Save(helper->ipsi, stm, TRUE);
	} else {
		res = get_persist_stream(helper);
		if (helper->ips) {
			res = IPersistStream_Save(helper->ips, stm, TRUE);
		}
	}
	
	IStream_Release(stm);

	if (FAILED(res)) {
		php_com_throw_exception(res, NULL TSRMLS_CC);
		return;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int COMPersistHelper::__construct([object com_object])
   Creates a persistence helper object, usually associated with a com_object */
CPH_METHOD(__construct)
{
	php_com_dotnet_object *obj = NULL;
	zval *zobj = NULL;
	CPH_FETCH();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|O!",
				&zobj, php_com_variant_class_entry)) {
		php_com_throw_exception(E_INVALIDARG, "invalid arguments" TSRMLS_CC);
		return;
	}

	if (!zobj) {
		return;
	}
	
	obj = CDNO_FETCH(zobj);

	if (V_VT(&obj->v) != VT_DISPATCH || V_DISPATCH(&obj->v) == NULL) {
		php_com_throw_exception(E_INVALIDARG, "parameter must represent an IDispatch COM object" TSRMLS_CC);
		return;
	}

	/* it is always safe to cast an interface to IUnknown */
	helper->unk = (IUnknown*)V_DISPATCH(&obj->v);
	IUnknown_AddRef(helper->unk);
	helper->codepage = obj->code_page;
}
/* }}} */




static const zend_function_entry com_persist_helper_methods[] = {
	CPH_ME(__construct, NULL)
	CPH_ME(GetCurFileName, NULL)
	CPH_ME(SaveToFile, NULL)
	CPH_ME(LoadFromFile, NULL)
	CPH_ME(GetMaxStreamSize, NULL)
	CPH_ME(InitNew, NULL)
	CPH_ME(LoadFromStream, NULL)
	CPH_ME(SaveToStream, NULL)
	PHP_FE_END
};

static void helper_free_storage(void *obj TSRMLS_DC)
{
	php_com_persist_helper *object = (php_com_persist_helper*)obj;

	if (object->ipf) {
		IPersistFile_Release(object->ipf);
	}
	if (object->ips) {
		IPersistStream_Release(object->ips);
	}
	if (object->ipsi) {
		IPersistStreamInit_Release(object->ipsi);
	}
	if (object->unk) {
		IUnknown_Release(object->unk);
	}
	zend_object_std_dtor(&object->std TSRMLS_CC);
	efree(object);
}


static void helper_clone(void *obj, void **clone_ptr TSRMLS_DC)
{
	php_com_persist_helper *clone, *object = (php_com_persist_helper*)obj;

	clone = emalloc(sizeof(*object));
	memcpy(clone, object, sizeof(*object));
	*clone_ptr = clone;

	zend_object_std_init(&clone->std, object->std.ce TSRMLS_CC);

	if (clone->ipf) {
		IPersistFile_AddRef(clone->ipf);
	}
	if (clone->ips) {
		IPersistStream_AddRef(clone->ips);
	}
	if (clone->ipsi) {
		IPersistStreamInit_AddRef(clone->ipsi);
	}
	if (clone->unk) {
		IUnknown_AddRef(clone->unk);
	}
}

static zend_object_value helper_new(zend_class_entry *ce TSRMLS_DC)
{
	php_com_persist_helper *helper;
	zend_object_value retval;

	helper = emalloc(sizeof(*helper));
	memset(helper, 0, sizeof(*helper));

	zend_object_std_init(&helper->std, helper_ce TSRMLS_CC);
	
	retval.handle = zend_objects_store_put(helper, NULL, helper_free_storage, helper_clone TSRMLS_CC);
	retval.handlers = &helper_handlers;

	return retval;
}

int php_com_persist_minit(INIT_FUNC_ARGS)
{
	zend_class_entry ce;

	memcpy(&helper_handlers, zend_get_std_object_handlers(), sizeof(helper_handlers));
	helper_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "COMPersistHelper", com_persist_helper_methods);
	ce.create_object = helper_new;
	helper_ce = zend_register_internal_class(&ce TSRMLS_CC);
	helper_ce->ce_flags |= ZEND_ACC_FINAL;

	le_istream = zend_register_list_destructors_ex(istream_dtor,
			NULL, "com_dotnet_istream_wrapper", module_number);
	
	return SUCCESS;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
