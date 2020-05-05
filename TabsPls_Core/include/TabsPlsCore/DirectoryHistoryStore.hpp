#pragma once

#include <stack>
#include "FileSystemDirectory.hpp"

struct ImpossibleSwitchException : std::exception {};

class DirectoryHistoryStore
{
public:
	void OnNewDirectory(const FileSystem::Directory&);

	void SwitchToPrevious();
	void SwitchToNext();

	const FileSystem::Directory& GetCurrent() const;

private:
	std::stack<FileSystem::Directory> m_previousDirs;
	std::stack<FileSystem::Directory> m_nextDirs;
};