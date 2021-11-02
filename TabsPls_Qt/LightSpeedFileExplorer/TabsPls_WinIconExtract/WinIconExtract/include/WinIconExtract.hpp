#pragma once

#include <string>
#include <optional>

#include <QPixmap>

namespace WinIconExtract {

void InitThread();

std::optional<QPixmap> IconInfoAsPixmap(const std::wstring& path);
} // namespace WinIconExtract