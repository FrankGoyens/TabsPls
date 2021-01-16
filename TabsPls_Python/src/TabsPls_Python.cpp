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

} // namespace TabsPlsPython