#include <TabsPls_Python/TabsPls_Python.hpp>

extern "C" {
#include <Python.h>
}

namespace TabsPlsPython {
void ExecPython(const char* code) {
    Py_Initialize();
    PyRun_SimpleString(code);
    Py_Finalize();
}

namespace {
struct AcquiredPyInstance {
    AcquiredPyInstance() { Py_Initialize(); }
    ~AcquiredPyInstance() { Py_Finalize(); }
};
struct AcquiredPyObject {
    AcquiredPyObject(PyObject* obj_) : obj(obj_) {}
    ~AcquiredPyObject() { Py_DECREF(obj); }
    PyObject* obj;
};
} // namespace

static void SendToTrashFromInitializedPy(const char* file) {
    AcquiredPyObject send2TrashModuleName = PyUnicode_FromString("send2trash");
    auto* send2TrashModule = PyImport_Import(send2TrashModuleName.obj);
    auto* send2TrashFunction = PyObject_GetAttrString(send2TrashModule, "send2trash");
    AcquiredPyObject args = PyTuple_Pack(1, PyUnicode_FromString(file));
    AcquiredPyObject result = PyObject_CallObject(send2TrashFunction, args.obj);
}

void SendToTrash(const char* file) {
    AcquiredPyInstance _;
    SendToTrashFromInitializedPy(file);
}
} // namespace TabsPlsPython