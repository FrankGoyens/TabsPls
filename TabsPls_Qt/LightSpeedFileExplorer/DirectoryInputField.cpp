#include "DirectoryInputField.hpp"

#include <stdexcept>

#include <QLineEdit>

#include "FileSystemDefsConversion.hpp"

#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/TargetDirectoryConstraints.hpp>

using FileSystem::StringConversion::ToRawPath;

DirectoryInputField::DirectoryInputField(QString initialDirectory) : m_currentDirectory(std::move(initialDirectory)) {
    if (!FileSystem::IsDirectory(ToRawPath(m_currentDirectory)))
        throw std::invalid_argument("The given directory is not valid");

    setText(m_currentDirectory);

    connect(this, &QLineEdit::editingFinished, [this]() {
        if (text() == m_currentDirectory)
            return;

        const auto newDirectory = ToRawPath(text());
        if (!FileSystem::IsDirectory(newDirectory)) {
            setText(m_currentDirectory);
            return;
        }

        if (TargetDirectoryConstraints::IsIncompleteWindowsRootPath(newDirectory)) {
            setText(m_currentDirectory);
            return;
        }

        m_currentDirectory = text();
        emit directoryChanged(m_currentDirectory);
    });
}