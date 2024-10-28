#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <TabsPlsCore/Toolbar.hpp>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <map>
#include <sstream>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

#define TABSPLSLOG_FUNC_INFO "<unknown function>"
#include <TabsPlsCore/TabsPlsLog.hpp>

#include "PythonRAII.hpp"

namespace TabsPlsPython::Toolbar {
bool operator==(const ToolbarItem& first, const ToolbarItem& second) {
    return first.id == second.id && first.displayName == second.displayName;
}

Toolbar::Toolbar(std::string id, std::vector<ToolbarItem> items) : m_id(std::move(id)), m_items(std::move(items)) {}

const std::string& Toolbar::GetId() const { return m_id; }

const std::vector<ToolbarItem>& Toolbar::GetItems() const { return m_items; }

bool operator==(const Toolbar& first, const Toolbar& second) {
    return first.m_id == second.m_id && first.m_items == second.m_items;
}

bool ComponentIsAvailable() { return true; }

void Init() {}

static std::string ToUTF8(const wchar_t* wideString) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wideString);
}

namespace {
struct PyPluginModule {
    PyObject* pyModule = nullptr;
    PyObject* pyActivationFunc = nullptr;
};

struct AcquiredModules {
    AcquiredModules(const std::string& modulePath, const std::vector<FileSystem::RawPath>& moduleNames) {
        std::ostringstream oss;
        oss << "import sys, os " << std::endl << R"(sys.path.append(r")" << modulePath.c_str() << R"("))" << std::endl;

        const auto runString = oss.str();
        PyRun_SimpleString(oss.str().c_str());

        for (const auto& moduleName : moduleNames) {
            AcquiredPyObject toolbarModuleName = PyUnicode_FromWideChar(moduleName.c_str(), moduleName.size());
            auto* module = PyImport_Import(toolbarModuleName.obj);

            if (module) {
                if (auto* activationFunc = PyObject_GetAttrString(module, "activate"))
                    modules.emplace(ToUTF8(moduleName.c_str()), PyPluginModule{module, activationFunc});
            } else {
                TabsPlsLog_WarningCategory("Pyton toolbar", "Unable to load toolbar module: %s", moduleName.c_str());
            }
        }
    }

    std::map<std::string, PyPluginModule> modules;
};
} // namespace

static std::vector<FileSystem::RawPath> FindModules(const FileSystem::Directory& pluginsDirectory) {
    std::vector<FileSystem::RawPath> pluginModules;
    for (const auto& path : FileSystem::GetFilesInDirectory(pluginsDirectory)) {
        if (path.find(L"Toolbar_") != FileSystem::RawPath::npos) {
            if (const auto saneFilePath = FileSystem::FilePath::FromPath(path))
                pluginModules.push_back(FileSystem::ReplaceExtension(FileSystem::GetFilename(*saneFilePath)));
        }
    }
    return pluginModules;
}

static AcquiredModules& GetPluginModules(const std::string& modulePath = {},
                                         const std::vector<FileSystem::RawPath>& moduleNames = {}) {
    static AcquiredModules modules(modulePath, moduleNames);
    return modules;
}

static ToolbarItem GetToolbarItem(PyObject& pyToolbarItemKey, PyObject& pyToolbarItem) {
    if (!PyDict_Check(&pyToolbarItem))
        throw ToolbarException("toolbar item is not a dictionary");

    ToolbarItem toolbarItem;
    AcquiredPyObject pyItemId = PyObject_Str(&pyToolbarItemKey);
    toolbarItem.id = PyUnicode_AsUTF8(pyItemId.obj);
    if (auto* pyDisplayName = PyDict_GetItemString(&pyToolbarItem, "display_name")) {
        AcquiredPyObject pyDisplayNameStr = PyObject_Str(pyDisplayName);
        toolbarItem.displayName = PyUnicode_AsUTF8(pyDisplayNameStr.obj);
    }
    return toolbarItem;
}

static std::vector<ToolbarItem> ReadItemsDict(PyObject& getItemsResult) {
    if (!PyDict_Check(&getItemsResult))
        throw ToolbarException("get_items did not return a dictionary");

    AcquiredPyObject keys = PyDict_Keys(&getItemsResult);
    const auto keyAmount = PyList_Size(keys.obj);
    if (keyAmount == 0)
        throw ToolbarException("get_items contains no values");

    std::vector<ToolbarItem> items;
    for (unsigned i = 0; i < keyAmount; ++i) {
        if (auto* toolbarItemKey = PyList_GetItem(keys.obj, i)) {
            items.push_back(GetToolbarItem(*toolbarItemKey, *PyDict_GetItem(&getItemsResult, toolbarItemKey)));
        }
    }
    return items;
}

static std::vector<ToolbarItem> GetItemsFromPlugin(PyObject& pluginModule) {
    if (auto* function = PyObject_GetAttrString(&pluginModule, "get_items")) {
        AcquiredPyObject result = PyObject_CallObject(function, nullptr);
        return ReadItemsDict(*result.obj);
    }
    return {};
}

std::vector<Toolbar> LoadToolbars(const FileSystem::Directory& pluginsDirectory) {
    const auto pluginsPathUTF8 = ToUTF8(pluginsDirectory.path().c_str());

    const auto moduleNames = FindModules(pluginsDirectory);
    const auto& pluginModules = GetPluginModules(pluginsPathUTF8, moduleNames);

    std::vector<Toolbar> toolbars;

    for (const auto& [moduleName, pyModule] : pluginModules.modules) {
        if (pyModule.pyModule) {
            auto toolbarItems = GetItemsFromPlugin(*pyModule.pyModule);
            if (!toolbarItems.empty()) {
                toolbars.emplace_back(moduleName, std::move(toolbarItems));
            }
        }
    }

    return toolbars;
}

static std::string StringFromPyObject(PyObject& result) {
    AcquiredPyObject pyDesiredReactionStr = PyObject_Str(&result);
    return PyUnicode_AsUTF8(pyDesiredReactionStr.obj);
}

namespace {
static ActivationResult GetResultFromPy(PyObject& result) {
    if (!PyTuple_Check(&result))
        return {};

    if (PyTuple_Size(&result) == 0)
        return {};

    ActivationResult activationResult;
    activationResult.desiredReaction = StringFromPyObject(*PyTuple_GetItem(&result, 0));

    if (PyTuple_Size(&result) < 2)
        return activationResult;

    activationResult.parameter = StringFromPyObject(*PyTuple_GetItem(&result, 1));
    return activationResult;
}

struct ActivatorImpl : Activator {
    std::set<std::string> GetToolbarNames() const override {
        std::set<std::string> names;
        const auto& modules = GetPluginModules();
        std::transform(modules.modules.begin(), modules.modules.end(), std::inserter(names, names.end()),
                       [](const auto& pair) { return pair.first; });
        return names;
    }
    ActivationResult Activate(const Toolbar& toolbar, const ToolbarItem& item,
                              ActivationMethod activationMethod) const override {
        const auto& pluginModules = GetPluginModules().modules;
        const auto it = pluginModules.find(toolbar.GetId());
        if (it == pluginModules.end())
            return {};
        AcquiredPyObject itemString = PyUnicode_FromString(item.id.c_str());
        PyObject* activationMethodStringPtr;
        switch (activationMethod) {
        case ActivationMethod::Regular:
            activationMethodStringPtr = PyUnicode_FromString("regular");
            break;
        case ActivationMethod::Alternative:
            activationMethodStringPtr = PyUnicode_FromString("alternative");
            break;
        }
        AcquiredPyObject activationMethodString(activationMethodStringPtr);
        AcquiredPyObject args = PyTuple_Pack(2, itemString.obj, activationMethodString.obj);
        AcquiredPyObject result = PyObject_CallObject(it->second.pyActivationFunc, args.obj);
        if (result.obj == Py_None || result.obj == nullptr)
            return {};
        return GetResultFromPy(*result.obj);
    }
};
} // namespace

ActivationResult Activate(const Toolbar& toolbar, const ToolbarItem& item, ActivationMethod activationMethod,
                          const Activator* customActivator) {
    ActivatorImpl activatorImpl;
    if (!customActivator)
        customActivator = &activatorImpl;

    const auto names = customActivator->GetToolbarNames();
    const auto it = names.find(toolbar.GetId());
    if (it == names.end())
        return {};

    const auto items = toolbar.GetItems();
    const auto itemIt = std::find(items.begin(), items.end(), item);

    if (itemIt == items.end())
        return {};

    return customActivator->Activate(toolbar, item, activationMethod);
}

} // namespace TabsPlsPython::Toolbar