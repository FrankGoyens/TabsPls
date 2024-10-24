#include <WinIconExtract.hpp>

#include <array>
#include <iostream>

#include <string.h>

#include <QtWinExtras>

#include <Windows.h>

namespace WinIconExtract {

void InitThread() {
    auto result = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (result != S_OK && result != S_FALSE) {
        std::cerr << "WinIconExtract: InitThread: CoInitializeEx error!" << std::endl;
    }
}

std::optional<QPixmap> IconInfoAsPixmap(const std::wstring& path) {
    QString sanitizedPath = QString::fromStdWString(path);
    sanitizedPath.replace("/", "\\");

    std::array<wchar_t, MAX_PATH> pathBuffer{};
    const auto sanitizedPathWString = sanitizedPath.toStdWString();
    const auto result = wcscpy_s(pathBuffer.data(), pathBuffer.size(), sanitizedPathWString.c_str());
    if (result != 0) {
        return {};
    }

    WORD index = 0;
    auto hIcon = ExtractAssociatedIconW(0, (LPWSTR)pathBuffer.data(), &index);
    if (hIcon != 0) {
        auto pixmap = QtWin::fromHICON(hIcon);
        DestroyIcon(hIcon);
        return pixmap;
    }
    return {};
}
} // namespace WinIconExtract
