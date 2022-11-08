#pragma once

namespace TabsPlsPython {
namespace EmbeddedPython {
bool ComponentIsAvailable();

//! \brief Call this from the main thread once
void Init(const char* programName);
} // namespace EmbeddedPython
} // namespace TabsPlsPython