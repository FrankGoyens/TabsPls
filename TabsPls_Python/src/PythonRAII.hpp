#pragma once

extern "C" {
#include <Python.h>
}

struct AcquiredPyInstance {
    AcquiredPyInstance() { Py_Initialize(); }
    ~AcquiredPyInstance() { Py_Finalize(); }
};

struct AcquiredPyObject {
    AcquiredPyObject(PyObject* obj_) : obj(obj_) {}
    ~AcquiredPyObject() { Py_DECREF(obj); }
    PyObject* obj;
};