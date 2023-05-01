#include <WinIconExtract.hpp>

#include <iostream>

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
    WORD index = 0;
    auto hIcon = ExtractAssociatedIconW(0, (LPWSTR)sanitizedPath.toStdWString().c_str(), &index);
    if (hIcon != 0) {
        auto pixmap = QtWin::fromHICON(hIcon);
        DestroyIcon(hIcon);
        return pixmap;
    }
    return {};
}
} // namespace WinIconExtract
