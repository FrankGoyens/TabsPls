#include <TabsPls_Python/TabsPls_Python.hpp>

#include "PythonRAII.hpp"

extern "C" {
#include <Python.h>
}

namespace TabsPlsPython {

void Init(const char* programName) { InitializePyInstance(programName); }

void ExecPython(const char* code) {
    Py_Initialize();
    PyRun_SimpleString(code);
    Py_Finalize();
}

} // namespace TabsPlsPython