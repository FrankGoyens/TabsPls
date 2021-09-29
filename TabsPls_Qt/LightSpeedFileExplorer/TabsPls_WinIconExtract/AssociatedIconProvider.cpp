#include "../AssociatedIconProvider.hpp"

#include <fstream>

#include <WinIconExtract.hpp>

AssociatedIconProvider& AssociatedIconProvider::Get() {
    static AssociatedIconProvider provider;
    return provider;
}

static auto DumpIcon(const FileSystem::RawPath& path) {
    struct AssociatedIconProviderDumper : WinIconExtract::IconDumper {
        void Dump(const std::vector<unsigned char>& data_, int width_, int height_) const override {
            data = data_;
            width = width_;
            height = height_;
        }
        mutable std::vector<unsigned char> data;
        mutable int width, height;
    } iconDumper;
    WinIconExtract::DumpAssociatedIconInfo(path, iconDumper);
    return std::make_tuple(iconDumper.data, iconDumper.width, iconDumper.height);
}

std::optional<QIcon> AssociatedIconProvider::FromPath(const FileSystem::RawPath& path) const {
    const auto [data, width, height] = DumpIcon(path);

    if (data.empty())
        return {};

    QPixmap pixmap;
    pixmap.loadFromData(data.data(), data.size(), "png");
    return QIcon(pixmap);
}

AssociatedIconProvider::AssociatedIconProvider() { WinIconExtract::Init(); }

AssociatedIconProvider::~AssociatedIconProvider() { WinIconExtract::DeInit(); }
