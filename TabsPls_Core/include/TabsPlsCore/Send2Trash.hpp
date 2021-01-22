#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

struct ProgressReport;

namespace TabsPlsPython {
// This is an interface to the 'Send2Trash' Python module. This can be used for sending files and folders to the trash
// on multiple platforms.
namespace Send2Trash {
bool ComponentIsAvailable();

struct Exception : std::exception {
    Exception(std::string message_) : message(std::move(message_)) {}
    const char* what() const noexcept override { return message.c_str(); };
    std::string message;
};
// \brief When the Python module send2trash can not be found
struct ModuleNotFoundException : Exception {
    ModuleNotFoundException(std::string message_) : Exception(std::move(message_)) {}
};

struct Result {
    std::optional<std::string> error;
};

Result SendToTrash(const char* item);

struct AggregatedResult {
    std::vector<std::pair<std::string, Result>> itemResults;
};
AggregatedResult SendToTrash(const std::vector<std::string>& items, const std::weak_ptr<ProgressReport>&);
} // namespace Send2Trash
} // namespace TabsPlsPython