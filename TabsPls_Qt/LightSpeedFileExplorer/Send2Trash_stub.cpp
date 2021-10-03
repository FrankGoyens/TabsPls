#include <TabsPlsCore/Send2Trash.hpp>

#include "ExplicitStub.hpp"

namespace TabsPlsPython {
namespace Send2Trash {

bool ComponentIsAvailable() { return false; }
Result SendToTrash(const char* item) { throw ExplicitStubException{}; }
AggregatedResult SendToTrash(const std::vector<std::string>& items,
                             const std::weak_ptr<ProgressReport>& progressReport) {
    throw ExplicitStubException{};
}

} // namespace Send2Trash
} // namespace TabsPlsPython