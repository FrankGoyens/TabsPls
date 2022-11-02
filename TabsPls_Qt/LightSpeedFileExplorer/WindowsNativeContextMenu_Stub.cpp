#include "WindowsNativeContextMenu.hpp"

#include "ExplicitStub.hpp"

namespace WindowsNativeContextMenu {
bool ComponentIsAvailable() { return false; }
bool ShowContextMenuForItems(const std::vector<std::wstring>&, int, int, void*) { throw ExplicitStubException{}; }

} // namespace WindowsNativeContextMenu
