#pragma once

#include <optional>
#include <string>

class QString;

struct DirectoryChanger {
    virtual ~DirectoryChanger() = default;

    //! \brief This will have no effect if the given directory does not exist
    virtual void ChangeDirectory(const QString&) = 0;

    /*! \brief This will have no effect if the given directory does not exist
     *
     * This should be called instead of 'ChangeDirectory' when the current
     * directory did not change.
     */
    virtual void RefreshDirectory(const QString&) = 0;

    virtual std::optional<std::string> ClaimError() = 0;
};