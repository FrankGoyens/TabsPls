#pragma once

#include <set>
#include <string>
#include <vector>

namespace FileSystem {
class Directory;
} // namespace FileSystem

namespace TabsPlsPython {
// This module is used to interface with Toolbars that are defined by users' python scripts
namespace Toolbar {

struct ToolbarException : std::exception {
    ToolbarException(std::string message) : message(std::move(message)) {}
    const char* what() const noexcept override { return message.c_str(); }
    std::string message;
};

struct ToolbarItem {
    std::string id;
    std::string displayName;
};

bool operator==(const ToolbarItem&, const ToolbarItem&);

class Toolbar {
  public:
    Toolbar(std::string id, std::vector<ToolbarItem> items);

    const std::string& GetId() const;
    const std::vector<ToolbarItem>& GetItems() const;

    friend bool operator==(const Toolbar&, const Toolbar&);

  private:
    std::string m_id;
    std::vector<ToolbarItem> m_items;
};

bool ComponentIsAvailable();

//! \brief Be sure to call InitializePyInstance first
void Init();

std::vector<Toolbar> LoadToolbars(const FileSystem::Directory& pluginsDirectory);

struct ActivationResult {
    std::string desiredReaction, parameter;
};

enum class ActivationMethod { Regular, Alternative };

struct Activator {
    virtual ~Activator() = default;
    virtual std::set<std::string> GetToolbarNames() const = 0;
    virtual ActivationResult Activate(const Toolbar& toolbar, const ToolbarItem& item,
                                      ActivationMethod activationMethod) const = 0;
};

[[nodiscard]] ActivationResult Activate(const Toolbar& toolbar, const ToolbarItem& item,
                                        ActivationMethod activationMethod = ActivationMethod::Regular,
                                        const Activator* customActivator = nullptr);

} // namespace Toolbar
} // namespace TabsPlsPython
