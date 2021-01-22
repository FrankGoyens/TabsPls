#include <TabsPlsCore/Send2Trash.hpp>

#include <TabsPlsCore/ProgressReport.hpp>

#include <algorithm>
#include <sstream>

#include "PythonRAII.hpp"

namespace TabsPlsPython {
namespace Send2Trash {

bool ComponentIsAvailable() { return true; }

namespace {

struct AcquiredSend2TrashModule {
    AcquiredSend2TrashModule() {
        AcquiredPyObject send2TrashModuleName = PyUnicode_FromString("send2trash");
        module = PyImport_Import(send2TrashModuleName.obj);

        if (module == nullptr)
            throw ModuleNotFoundException("The Python environment could not load module 'send2trash'. Or the function "
                                          "'send2trash.send2trash' could not be found.");
    }

    PyObject* module = nullptr;
};

} // namespace
static PyObject* GetSend2TrashModule() {
    static AcquiredSend2TrashModule acquiredModule;
    return acquiredModule.module;
}

namespace {
struct AcquiredSend2TrashFunction {
    AcquiredSend2TrashFunction() {
        function = PyObject_GetAttrString(GetSend2TrashModule(), "send2trash");

        if (function == nullptr)
            throw ModuleNotFoundException("The function 'send2trash.send2trash' could not be found.");
    }
    PyObject* function = nullptr;
};
} // namespace

static PyObject* GetSend2TrashFunction() {
    static AcquiredSend2TrashFunction acquiredFunction;
    return acquiredFunction.function;
}

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

static std::optional<std::string> CallSend2TrashFunction(const char* item) {
    AcquiredPyObject itemString = PyUnicode_FromString(item);
    AcquiredPyObject args = PyTuple_Pack(1, itemString.obj);
    AcquiredPyObject result = PyObject_CallObject(GetSend2TrashFunction(), args.obj);

    return RetrieveMessageIfExceptionOcurred();
}

template <typename ProgressWithValue> static void UpdateProgress(ProgressWithValue& progressWithValue) {
    if (const auto liveProgress = progressWithValue.first.lock())
        liveProgress->UpdateValue(++progressWithValue.second);
}

// \brief Returns an error string when an error ocurred
static AggregatedResult SendMultipleToTrashFromInitializedPy(const std::vector<std::string>& items,
                                                             const std::weak_ptr<ProgressReport>& progressReport) {
    AcquiredPyObject send2TrashModuleName = PyUnicode_FromString("send2trash");

    auto progressWithValue = std::make_pair(progressReport, 0);

    AggregatedResult result;
    std::transform(items.begin(), items.end(), std::back_inserter(result.itemResults), [&](const auto& item) {
        UpdateProgress(progressWithValue);
        return std::make_pair(item, Result{CallSend2TrashFunction(item.c_str())});
    });
    return result;
}

Result SendToTrash(const char* item) {
    InitializePyInstance();
    if (auto error = CallSend2TrashFunction(item))
        return {*error};
    return {};
}

static void StartProgress(const std::vector<std::string>& items, const std::weak_ptr<ProgressReport>& progressReport) {
    if (auto liveProgressReport = progressReport.lock()) {
        liveProgressReport->SetMinimum(0);
        liveProgressReport->SetMaximum(items.size());
        liveProgressReport->UpdateValue(0);
    }
}

AggregatedResult SendToTrash(const std::vector<std::string>& items,
                             const std::weak_ptr<ProgressReport>& progressReport) {
    InitializePyInstance();
    StartProgress(items, progressReport);
    return SendMultipleToTrashFromInitializedPy(items, progressReport);
}
} // namespace Send2Trash
} // namespace TabsPlsPython