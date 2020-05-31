#pragma once

#include "DirectoryHistoryStore.hpp"

/*! \brief A 'robust' layer around the normal DirectoryHistoryStore. Robust here means that on every call, the existance of the directory is verified. */
class RobustDirectoryHistoryStore
{
public:
	/*! \brief This function still trusts that the given directory still exists*/
	void OnNewDirectory(const FileSystem::Directory&);

	void SwitchToPrevious();
	void SwitchToNext();

	const FileSystem::Directory GetCurrent() const;

private:
	DirectoryHistoryStore m_store;
};