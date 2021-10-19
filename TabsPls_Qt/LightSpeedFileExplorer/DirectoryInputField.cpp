#include "DirectoryInputField.hpp"

#include <stdexcept>

#include <QLineEdit>

#include "FileSystemDefsConversion.hpp"

#include <TabsPlsCore/FileSystem.hpp>

using FileSystem::StringConversion::ToRawPath;

static bool DirIsRoot(const QString& dir) {
    const auto rawDir = ToRawPath(dir);
    return rawDir == FileSystem::_getRootPath(rawDir);
}

static QString RemoveWhitespace(QString string) {
    string = string.simplified();
    return string.replace(" ", "");
}

//! \brief For some reason, paths like "C:" are accepted, but iterating through them yiels the current directory's
//! contents. Very strange.
static bool IsIncompleteWindowsRootPath(QString path) {
    path = RemoveWhitespace(path);
    return DirIsRoot(path) && path.endsWith(':');
}

DirectoryInputField::DirectoryInputField(QString initialDirectory) : m_currentDirectory(std::move(initialDirectory)) {
    if (!FileSystem::IsDirectory(ToRawPath(m_currentDirectory)))
        throw std::invalid_argument("The given directory is not valid");

    setText(m_currentDirectory);

    connect(this, &QLineEdit::editingFinished, [this]() {
        if (text() == m_currentDirectory)
            return;

        const auto newDirectory = text();
        if (!FileSystem::IsDirectory(ToRawPath(newDirectory))) {
            setText(m_currentDirectory);
            return;
        }

        if (IsIncompleteWindowsRootPath(newDirectory)) {
            setText(m_currentDirectory);
            return;
        }

        m_currentDirectory = text();
        emit directoryChanged(m_currentDirectory);
    });
}