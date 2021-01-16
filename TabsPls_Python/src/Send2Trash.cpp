#include <TabsPls_Python/Send2Trash.hpp>

#include "PythonRAII.hpp"

namespace TabsPlsPython {
namespace Send2Trash {
static void SendToTrashFromInitializedPy(const char* item) {
    AcquiredPyObject send2TrashModuleName = PyUnicode_FromString("send2trash");
    auto* send2TrashModule = PyImport_Import(send2TrashModuleName.obj);
    auto* send2TrashFunction = PyObject_GetAttrString(send2TrashModule, "send2trash");
    AcquiredPyObject args = PyTuple_Pack(1, PyUnicode_FromString(item));
    AcquiredPyObject result = PyObject_CallObject(send2TrashFunction, args.obj);
}

void SendToTrash(const char* item) {
    AcquiredPyInstance _;
    SendToTrashFromInitializedPy(item);
}
} // namespace Send2Trash
} // namespace TabsPlsPython