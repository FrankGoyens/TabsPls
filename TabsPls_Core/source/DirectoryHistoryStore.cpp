#include "pch.h"

#include <TabsPlsCore/DirectoryHistoryStore.hpp>

void DirectoryHistoryStore::OnNewDirectory(const FileSystem::Directory& dir)
{
	m_previousDirs.push(dir);
	m_nextDirs = std::stack<FileSystem::Directory>();
}

void DirectoryHistoryStore::SwitchToPrevious()
{
	if (m_previousDirs.size() <= 1)
		throw ImpossibleSwitchException();

	m_nextDirs.push(m_previousDirs.top());
	m_previousDirs.pop();
}

void DirectoryHistoryStore::SwitchToNext()
{
	if (m_nextDirs.empty())
		throw ImpossibleSwitchException();

	m_previousDirs.push(m_nextDirs.top());
	m_nextDirs.pop();
}

const FileSystem::Directory DirectoryHistoryStore::GetCurrent() const
{
	if (m_previousDirs.empty())
		throw StoreIsEmptyException();

	return m_previousDirs.top();
}
