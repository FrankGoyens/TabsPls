#pragma once

#include <string>
#include <vector>

namespace WinIconExtract {
void Init();
void DeInit();

struct IconDumper {
    virtual ~IconDumper() = default;

    virtual void Dump(const std::vector<unsigned char>& data, int width, int height) const = 0;
};

void DumpAssociatedIconInfo(const std::string& path, const IconDumper&);
void DumpIconInfo(const std::string& path, const IconDumper&);
} // namespace WinIconExtract