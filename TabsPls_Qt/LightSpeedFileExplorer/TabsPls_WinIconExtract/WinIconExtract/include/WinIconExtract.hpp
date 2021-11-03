#pragma once

#include <optional>
#include <string>

#include <QPixmap>

namespace WinIconExtract {

void InitThread();

std::optional<QPixmap> IconInfoAsPixmap(const std::wstring& path);
} // namespace WinIconExtract