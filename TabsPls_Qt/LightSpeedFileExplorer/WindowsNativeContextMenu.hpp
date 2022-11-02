#pragma once

#include <string>
#include <vector>

namespace WindowsNativeContextMenu {
bool ComponentIsAvailable();

bool ShowContextMenuForItems(const std::vector<std::wstring>& absolutePaths, int x, int y, void* parentWindow);
} // namespace WindowsNativeContextMenu