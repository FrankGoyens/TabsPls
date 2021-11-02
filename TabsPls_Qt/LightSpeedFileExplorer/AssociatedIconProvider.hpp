#pragma once

#include <optional>

#include <FileSystemDefs.hpp>

#include <QIcon>

class AssociatedIconProvider final {
  public:
    static bool ComponentIsAvailable();
    static AssociatedIconProvider& Get();

    //Call this at least once from any thread that will retrieve icons
    static void InitThread();

    ~AssociatedIconProvider();

    std::optional<QIcon> FromPath(const FileSystem::RawPath&) const;

  private:
    AssociatedIconProvider();
};