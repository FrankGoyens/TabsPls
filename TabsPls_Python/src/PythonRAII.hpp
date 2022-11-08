#pragma once

extern "C" {
#include <Python.h>
}

// Don't use this directly, always use InitializeProcessWidePy
struct AcquiredPyInstance {
    AcquiredPyInstance(const char* programName) {
        program = Py_DecodeLocale(programName, NULL);

        if (program == NULL) {
            fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
            exit(1);
        }
        Py_SetProgramName(program);

        Py_Initialize();
    }
    ~AcquiredPyInstance() {
        Py_Finalize();
        PyMem_RawFree(program);
    }
    wchar_t* program;
};

// Initializes Python, subsequent calls are no-op
inline void InitializePyInstance(const char* programName) { static AcquiredPyInstance _(programName); }

struct AcquiredPyObject {
    AcquiredPyObject(PyObject* obj_ = nullptr) : obj(obj_) {}
    ~AcquiredPyObject() {
        if (obj != nullptr && obj != Py_None)
            Py_DECREF(obj);
    }
    operator bool() { return obj != nullptr; }
    PyObject* obj;
};