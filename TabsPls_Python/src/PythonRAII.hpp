#pragma once

#include <Python.h>

//Don't use this directly, always use InitializeProcessWidePy
struct AcquiredPyInstance {
    AcquiredPyInstance() { Py_Initialize(); }
    ~AcquiredPyInstance() { Py_Finalize(); }
};

//Initializes Python, subsequent calls are no-op
void InitializePyInstance() { static AcquiredPyInstance _; }

struct AcquiredPyObject {
    AcquiredPyObject(PyObject* obj_ = nullptr) : obj(obj_) {}
    ~AcquiredPyObject() {
        if (obj != nullptr && obj != Py_None)
            Py_DECREF(obj);
    }
    operator bool() { return obj != nullptr; }
    PyObject* obj;
};