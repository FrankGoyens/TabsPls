#include "CurrentDirectoryFileOpQtImpl.hpp"

CurrentDirectoryFileOpQtImpl::CurrentDirectoryFileOpQtImpl(FileSystem::Directory currentDir)
    : m_currentDir(std::move(currentDir)) {}

void CurrentDirectoryFileOpQtImpl::updateCurrentDir(FileSystem::Directory dir) { m_currentDir = std::move(dir); }

FileSystem::Directory CurrentDirectoryFileOpQtImpl::GetCurrentDir() const { return m_currentDir; }
