#pragma once

#include <string>
#include <vector>

namespace WinIconExtract {
void Init();
void DeInit();

struct IconDumper {
    virtual ~IconDumper() = default;

    virtual void Dump(std::vector<unsigned char> data, int width, int height) const = 0;
};

void DumpAssociatedIconInfo(const std::wstring& path, const IconDumper&);
void DumpIconInfo(const std::wstring& path, const IconDumper&);
} // namespace WinIconExtract