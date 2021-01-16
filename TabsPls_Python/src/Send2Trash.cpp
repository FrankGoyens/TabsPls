#include <TabsPls_Python/Send2Trash.hpp>

#include <algorithm>
#include <sstream>

#include "PythonRAII.hpp"

namespace TabsPlsPython {
namespace Send2Trash {

static std::string RetreiveMessageFromPyObjects(const AcquiredPyObject& exceptionType,
                                                const AcquiredPyObject& exceptionMessage) {
    std::ostringstream message;
    message << PyUnicode_AsUTF8(exceptionType.obj) << ": " << PyUnicode_AsUTF8(exceptionMessage.obj);
    return message.str();
}

static std::optional<std::string> RetrieveMessageIfExceptionOcurred() {
    if (AcquiredPyObject send2trashException = PyErr_Occurred()) {
        AcquiredPyObject exceptionType, exceptionValue, exceptionTraceback;
        PyErr_Fetch(&exceptionType.obj, &exceptionValue.obj, &exceptionTraceback.obj);
        return RetreiveMessageFromPyObjects(exceptionType, exceptionValue);
    }

    return {};
}

static PyObject* LoadSend2TrashFunction(const AcquiredPyObject& send2TrashModuleName) {
    auto* send2TrashModule = PyImport_Import(send2TrashModuleName.obj);

    if (send2TrashModule == nullptr)
        throw ModuleNotFoundException("The Python environment could not load module 'send2trash'. Or the function "
                                      "'send2trash.send2trash' could not be found.");

    auto* send2TrashFunction = PyObject_GetAttrString(send2TrashModule, "send2trash");

    if (send2TrashFunction == nullptr)
        throw ModuleNotFoundException("The function 'send2trash.send2trash' could not be found.");

    return send2TrashFunction;
}

static std::optional<std::string> CallSend2TrashFunction(PyObject* send2TrashFunction, const char* item) {
    AcquiredPyObject args = PyTuple_Pack(1, PyUnicode_FromString(item));
    AcquiredPyObject result = PyObject_CallObject(send2TrashFunction, args.obj);

    return RetrieveMessageIfExceptionOcurred();
}

// \brief Returns an error string when an error ocurred
static std::optional<std::string> SendToTrashFromInitializedPy(const char* item) {
    AcquiredPyObject send2TrashModuleName = PyUnicode_FromString("send2trash");

    auto* send2TrashFunction = LoadSend2TrashFunction(send2TrashModuleName);

    return CallSend2TrashFunction(send2TrashFunction, item);
}

// \brief Returns an error string when an error ocurred
static AggregatedResult SendMultipleToTrashFromInitializedPy(std::vector<std::string> items) {
    AcquiredPyObject send2TrashModuleName = PyUnicode_FromString("send2trash");

    auto* send2TrashFunction = LoadSend2TrashFunction(send2TrashModuleName);

    AggregatedResult result;
    std::transform(items.begin(), items.end(), std::back_inserter(result.failedItems), [&](const auto& item) {
        return std::make_pair(item, Result{CallSend2TrashFunction(send2TrashFunction, item.c_str())});
    });
    return result;
}

Result SendToTrash(const char* item) {
    AcquiredPyInstance _;
    if (auto error = SendToTrashFromInitializedPy(item))
        return {*error};
    return {};
}

AggregatedResult SendToTrash(std::vector<std::string> items) {
    AcquiredPyInstance _;
    return SendMultipleToTrashFromInitializedPy(items);
}
} // namespace Send2Trash
} // namespace TabsPlsPython