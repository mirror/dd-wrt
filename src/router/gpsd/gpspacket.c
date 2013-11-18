/* $Id: gpspacket.c 4102 2006-12-07 15:18:18Z esr $ */
/*
 * Python binding for the packet.c module.
 */
#include <Python.h>

#include <stdio.h>
#include "gpsd_config.h"
#include "gpsd.h"

static PyObject *ErrorObject = NULL;

static PyObject *report_callback = NULL;

void gpsd_report(int errlevel UNUSED, const char *fmt, ... )
{
    char buf[BUFSIZ];
    PyObject *args;
    va_list ap;

    if (!report_callback)   /* no callback defined, exit early */
	return;	
    
    if (!PyCallable_Check(report_callback)) {
	PyErr_SetString(ErrorObject, "Cannot call Python callback function");
	return;
    }

    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    args = Py_BuildValue("(is)", errlevel, buf);
    if (!args)
	return;

    PyObject_Call(report_callback, args, NULL);
    Py_DECREF(args);
}

static PyTypeObject Lexer_Type;

typedef struct {
	PyObject_HEAD
	struct gps_packet_t lexer;
} LexerObject;

static LexerObject *
newLexerObject(PyObject *arg)
{
    LexerObject *self;
    self = PyObject_New(LexerObject, &Lexer_Type);
    if (self == NULL)
	return NULL;
    memset(&self->lexer, 0, sizeof(struct gps_packet_t));
    packet_reset(&self->lexer);
    return self;
}

/* Lexer methods */

static int
Lexer_init(LexerObject *self)
{
    packet_reset(&self->lexer);
    return 0;
}
static PyObject *
Lexer_get(LexerObject *self, PyObject *args)
{
    ssize_t len;
    int fd;

    if (!PyArg_ParseTuple(args, "i;missing or invalid file descriptor argument to gpspacket.get", &fd))
        return NULL;

    len = packet_get(fd, &self->lexer);
    if (PyErr_Occurred())
	return NULL;

    if (len == 0) {
	self->lexer.type = BAD_PACKET;
	self->lexer.outbuffer[0] = '\0';
    }

    return Py_BuildValue("(i, s)", self->lexer.type, self->lexer.outbuffer);
}

static PyObject *
Lexer_reset(LexerObject *self)
{
    packet_reset(&self->lexer);
    if (PyErr_Occurred())
	return NULL;
    return 0;
}

static void
Lexer_dealloc(LexerObject *self)
{
    PyObject_Del(self);
}

static PyMethodDef Lexer_methods[] = {
    {"get",	(PyCFunction)Lexer_get,	METH_VARARGS,
    		PyDoc_STR("Get a packet from a file descriptor.")},
    {"reset",	(PyCFunction)Lexer_reset,	METH_NOARGS,
    		PyDoc_STR("Reset the packet lexer to ground state.")},
    {NULL,		NULL}		/* sentinel */
};

static PyObject *
Lexer_getattr(LexerObject *self, char *name)
{
    return Py_FindMethod(Lexer_methods, (PyObject *)self, name);
}

PyDoc_STRVAR(Lexer__doc__,
"GPS packet lexer object\n\
\n\
Fetch a single packet from file descriptor");

static PyTypeObject Lexer_Type = {
	/* The ob_type field must be initialized in the module init function
	 * to be portable to Windows without using C++. */
	PyObject_HEAD_INIT(NULL)
	0,			/*ob_size*/
	"gpspacket.lexer",	/*tp_name*/
	sizeof(LexerObject),	/*tp_basicsize*/
	0,			/*tp_itemsize*/
	/* methods */
	(destructor)Lexer_dealloc, /*tp_dealloc*/
	0,			/*tp_print*/
	(getattrfunc)Lexer_getattr,			/*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	0,			/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
        0,                      /*tp_call*/
        0,                      /*tp_str*/
        0,                      /*tp_getattro*/
        0,                      /*tp_setattro*/
        0,                      /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT,     /*tp_flags*/
        Lexer__doc__,          /*tp_doc*/
        0,                      /*tp_traverse*/
        0,                      /*tp_clear*/
        0,                      /*tp_richcompare*/
        0,                      /*tp_weaklistoffset*/
        0,                      /*tp_iter*/
        0,                      /*tp_iternext*/
        Lexer_methods,		/*tp_methods*/
        0,                      /*tp_members*/
        0,                      /*tp_getset*/
        0,                      /*tp_base*/
        0,                      /*tp_dict*/
        0,                      /*tp_descr_get*/
        0,                      /*tp_descr_set*/
        0,                      /*tp_dictoffset*/
        (initproc)Lexer_init,	/*tp_init*/
        0,                      /*tp_alloc*/
        0,                      /*tp_new*/
        0,                      /*tp_free*/
        0,                      /*tp_is_gc*/
};

/* Function of no arguments returning new Lexer object */

static PyObject *
gpspacket_new(PyObject *self, PyObject *args)
{
    LexerObject *rv;

    if (!PyArg_ParseTuple(args, ":new"))
	return NULL;
    rv = newLexerObject(args);
    if (rv == NULL)
	return NULL;
    return (PyObject *)rv;
}

PyDoc_STRVAR(register_report__doc__,
"register_report(callback)\n\
\n\
callback must be a callable object expecting a string as parameter.");

static PyObject *
register_report(LexerObject *self, PyObject *args)
{
    PyObject *callback = NULL;

    if (!PyArg_ParseTuple(args, "O:register_report", &callback))
	return NULL;

    if (!PyCallable_Check(callback)) {
	PyErr_SetString(PyExc_TypeError, "First argument must be callable");
	return NULL;
    }

    if (report_callback) {
	Py_DECREF(report_callback);
	report_callback = NULL;
    }

    report_callback = callback;
    Py_INCREF(report_callback);

    Py_INCREF(Py_None);
    return Py_None;
}


/* List of functions defined in the module */

static PyMethodDef gpspacket_methods[] = {
    {"new",		gpspacket_new,		METH_VARARGS,
     PyDoc_STR("new() -> new packet-lexer object")},
    {"register_report", (PyCFunction)register_report, METH_VARARGS,
			register_report__doc__},
    {NULL,		NULL}		/* sentinel */
};

PyDoc_STRVAR(module_doc,
"Python binding of the libgpsd module for recognizing GPS packets.\n\
The new() function returns a new packet-lexer instance.  Lexer instances\n\
have two methods:\n\
    get() takes a file descriptor argument and returns a tuple consisting of\n\
the integer packet type and string packet value.  On end of file it returns\n\
(-1, '').\n\
    reset() resets the packet-lexer to its initial state.\n\
    The module also has a register_report() function that accepts a callback\n\
for debug message reporting.  The callback will get two arguments, the error\n\
level of the message and the message itself.\n\
");

PyMODINIT_FUNC
initgpspacket(void)
{
    PyObject *m;

    if (PyType_Ready(&Lexer_Type) < 0)
	return;

    /* Create the module and add the functions */
    m = Py_InitModule3("gpspacket", gpspacket_methods, module_doc);

    PyModule_AddIntConstant(m, "BAD_PACKET", BAD_PACKET);
    PyModule_AddIntConstant(m, "COMMENT_PACKET", COMMENT_PACKET);
    PyModule_AddIntConstant(m, "NMEA_PACKET", NMEA_PACKET);
    PyModule_AddIntConstant(m, "SIRF_PACKET", SIRF_PACKET);
    PyModule_AddIntConstant(m, "ZODIAC_PACKET", ZODIAC_PACKET);
    PyModule_AddIntConstant(m, "TSIP_PACKET", TSIP_PACKET);
    PyModule_AddIntConstant(m, "EVERMORE_PACKET", EVERMORE_PACKET);
    PyModule_AddIntConstant(m, "ITALK_PACKET", ITALK_PACKET);
    PyModule_AddIntConstant(m, "RTCM_PACKET", RTCM_PACKET);
    PyModule_AddIntConstant(m, "GARMIN_PACKET", GARMIN_PACKET);
}
