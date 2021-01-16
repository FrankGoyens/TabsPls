#pragma once

namespace TabsPlsPython {
// This is an interface to the 'Send2Trash' Python module. This can be used for sending files and folders to the trash on multiple
// platforms.
namespace Send2Trash {
void SendToTrash(const char* item);
}
} // namespace TabsPlsPython