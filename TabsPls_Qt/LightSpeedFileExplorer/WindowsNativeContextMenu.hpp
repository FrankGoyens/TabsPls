#pragma once

#include <string>

namespace WindowsNativeContextMenu {
inline bool ComponentIsAvailable() { return true; }

bool ShowContextMenuForItem(const std::wstring& absolutePath, int x, int y, void* parentWindow);
} // namespace WindowsNativeContextMenu