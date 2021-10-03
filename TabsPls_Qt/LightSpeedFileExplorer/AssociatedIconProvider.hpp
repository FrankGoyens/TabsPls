#pragma once

#include <optional>

#include <FileSystemDefs.hpp>

#include <QIcon>

class AssociatedIconProvider final {
  public:
    static bool ComponentIsAvailable();
    static AssociatedIconProvider& Get();

    std::optional<QIcon> FromPath(const FileSystem::RawPath&) const;

  private:
    AssociatedIconProvider();
    ~AssociatedIconProvider();
};