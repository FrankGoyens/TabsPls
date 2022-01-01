#include "../AssociatedIconProvider.hpp"

#include <QGuiApplication>

#include <WinIconExtract.hpp>

bool AssociatedIconProvider::ComponentIsAvailable() { return true; }

AssociatedIconProvider& AssociatedIconProvider::Get() {
    static AssociatedIconProvider provider;
    return provider;
}

std::optional<QIcon> AssociatedIconProvider::FromPath(const FileSystem::RawPath& path) const {
    if (QGuiApplication::instance() == nullptr)
        return {};
    if (const auto pixmap = WinIconExtract::IconInfoAsPixmap(path))
        return QIcon(*pixmap);
    return {};
}

AssociatedIconProvider::AssociatedIconProvider() = default;

void AssociatedIconProvider::InitThread() { WinIconExtract::InitThread(); }

AssociatedIconProvider::~AssociatedIconProvider() = default;
