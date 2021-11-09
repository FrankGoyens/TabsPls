#include "DirectoryInputField.hpp"

#include <set>
#include <stdexcept>

#include <QKeyEvent>
#include <QLineEdit>

#include "FileSystemDefsConversion.hpp"

#include <TabsPlsCore/DirectoryInputAutoComplete.hpp>
#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/TargetDirectoryConstraints.hpp>

using FileSystem::StringConversion::FromRawPath;
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

void DirectoryInputField::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Tab) {
        const auto incompletePath = ToRawPath(text());

        if (const auto autoCompleter = DirectoryInputAutoComplete::Do(incompletePath)) {
            setText(FromRawPath(*autoCompleter));
        }
        return;
    }
    QLineEdit::keyPressEvent(event);
}
