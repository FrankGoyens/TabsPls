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

void SendToTrash(const char* file) {
    Py_Initialize();
    auto* send2TrashModuleName = PyUnicode_FromString("send2trash");
    auto* send2TrashModule = PyImport_Import(send2TrashModuleName);
    auto* send2TrashFunction = PyObject_GetAttrString(send2TrashModule, "send2trash");
    auto* args = PyTuple_Pack(1, PyUnicode_FromString(file));
    auto* result = PyObject_CallObject(send2TrashFunction, args);
    Py_Finalize();
}
} // namespace TabsPlsPython