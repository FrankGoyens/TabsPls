#include <TabsPlsCore/EmbeddedPython.hpp>

#include "ExplicitStub.hpp"

namespace TabsPlsPython::EmbeddedPython {
bool ComponentIsAvailable() { return false; }

void Init(const char*) { throw ExplicitStubException(); }
} // namespace TabsPlsPython::EmbeddedPython
