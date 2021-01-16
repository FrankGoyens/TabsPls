#pragma once

extern "C" {
#include <Python.h>
}

struct AcquiredPyInstance {
    AcquiredPyInstance() { Py_Initialize(); }
    ~AcquiredPyInstance() { Py_Finalize(); }
};

struct AcquiredPyObject {
    AcquiredPyObject(PyObject* obj_ = nullptr) : obj(obj_) {}
    ~AcquiredPyObject() {
        if (obj)
            Py_DECREF(obj);
    }
    operator bool() { return obj != nullptr; }
    PyObject* obj;
};