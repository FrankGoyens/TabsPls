#include <TabsPlsCore/Send2Trash.hpp>

#include "ExplicitStub.hpp"

namespace TabsPlsPython {
namespace Send2Trash {

bool ComponentIsAvailable() { return false; }
Result SendToTrash(const char* item) { throw ExplicitStubException{}; }
AggregatedResult SendToTrash(std::vector<std::string> items) { throw ExplicitStubException{}; }

} // namespace Send2Trash
} // namespace TabsPlsPython