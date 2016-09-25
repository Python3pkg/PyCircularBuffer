#include "base.h"
#include "mapping.h"
#include "methods.h"
#include "sequence.h"

/* magic methods */

PyObject* CircularBuffer_create(PyTypeObject* type, PyObject* args,
        PyObject* kwargs)
{
    CircularBuffer* self;

    self = (CircularBuffer*) type->tp_alloc(type, 0);
    if (self)
    {
        self->raw = NULL;
        self->read = 0;
        self->write = 0;
        self->allocated = 0;
        self->allocated_before_resize = 0;
    }

    return (PyObject*) self;
}

int CircularBuffer_initialize(CircularBuffer* self, PyObject* args,
        PyObject* kwargs)
{
    static char* kwlist[] = {"size", NULL};

    Py_ssize_t size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "n", kwlist, &size))
    {
        // Exception already set by PyArg_ParseTuple
        return -1;
    }

    self->raw = (char*) PyMem_Malloc(size + 2);
    if (self->raw == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    self->allocated = size;
    self->allocated_before_resize = size;
    self->raw[0] = 0;
    self->raw[size + 1] = 0;
    return 0;
}

void CircularBuffer_destroy(CircularBuffer* self)
{
    PyMem_Free(self->raw);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

PyObject* CircularBuffer_repr(CircularBuffer* self)
{
    if (self->write >= self->read)
    {
        return PyString_FromFormat("<CircularBuffer[%u]:%s>",
                circularbuffer_total_length(self),
                circularbuffer_readptr(self));
    }
    else
    {
        return PyString_FromFormat("<CircularBuffer[%u]:%s%s>",
                circularbuffer_total_length(self),
                circularbuffer_readptr(self),
                self->raw);
    }
}

PyObject* CircularBuffer_str(CircularBuffer* self)
{
    if (self->write >= self->read)
    {
        return PyString_FromFormat("%s", circularbuffer_readptr(self));
    }
    else
    {
        return PyString_FromFormat("%s%s", circularbuffer_readptr(self),
                self->raw);
    }
}


/* meta description */


PyMethodDef Module_methods[] = {
    // end of array
    {NULL},
};

// see: https://docs.python.org/2/c-api/typeobj.html

PyTypeObject CircularBufferType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "circularbuffer.CircularBuffer",           // tp_name
    sizeof(CircularBuffer),                    // tp_basicsize
    0,                                         // tp_itemsize
    (destructor) CircularBuffer_destroy,       // tp_dealloc
    0,                                         // tp_print (deprecated)
    0,                                         // tp_getattr (deprecated)
    0,                                         // tp_setattr (deprecated)
    0,                                         // tp_compare
    (reprfunc) CircularBuffer_repr,            // tp_repr
    0,                                         // tp_as_number
    CircularBuffer_sequence,                   // tp_as_sequence
    CircularBuffer_mapping,                    // tp_as_mapping
    0,                                         // tp_hash
    0,                                         // tp_call
    (reprfunc) CircularBuffer_str,             // tp_str
    0,                                         // tp_getattro
    0,                                         // tp_setattro
    0,                                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
    "Circular buffer",                         // tp_doc
    0,                                         // tp_traverse
    0,                                         // tp_clear
    0,                                         // tp_richcompare
    0,                                         // tp_weaklistoffset
    0,                                         // tp_iter
    0,                                         // tp_iternext
    CircularBuffer_methods,                    // tp_methods
    0,                                         // tp_members
    0,                                         // tp_getset
    0,                                         // tp_base
    0,                                         // tp_dict
    0,                                         // tp_descr_get
    0,                                         // tp_descr_set
    0,                                         // tp_dictoffset
    (initproc) CircularBuffer_initialize,      // tp_init
    0,                                         // tp_alloc
    CircularBuffer_create,                     // tp_new
};

#if PY_MAJOR_VERSION >= 3
struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "circularbuffer",                           /* m_name */
    "Simple implementation of circular bufer.", /* m_doc */
    -1,                                         /* m_size */
    Module_methods,                             /* m_methods */
    NULL,                                       /* m_reload */
    NULL,                                       /* m_traverse */
    NULL,                                       /* m_clear */
    NULL,                                       /* m_free */
};
#endif


/* initialization */

PyObject *module_init(void)
{
    // create new module
    #if PY_MAJOR_VERSION >= 3
    PyObject* module = PyModule_Create(&moduledef);
    #else
    PyObject* module = Py_InitModule3("circularbuffer", Module_methods,
        "Simple implementation of circular buffer."
    );
    #endif
    if (module == NULL) { return NULL; }

    // create new class
    if (PyType_Ready(&CircularBufferType) < 0) { return NULL; }

    // add class to module
    Py_INCREF(&CircularBufferType);
    PyModule_AddObject(module, "CircularBuffer",
                       (PyObject*) &CircularBufferType);

    return module;
}

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initcircularbuffer(void) {
    module_init();
}
#else
PyMODINIT_FUNC PyInit_circularbuffer(void) {
    return module_init();
}
#endif
