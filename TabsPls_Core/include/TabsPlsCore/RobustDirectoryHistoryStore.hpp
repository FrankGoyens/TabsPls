#pragma once

#include "DirectoryHistoryStore.hpp"

/*! \brief A 'robust' layer around the normal DirectoryHistoryStore. Robust here means that on every call, the existance of the directory is verified. */
class RobustDirectoryHistoryStore
{
public:
	RobustDirectoryHistoryStore() = default;
	~RobustDirectoryHistoryStore() = default;

	RobustDirectoryHistoryStore(RobustDirectoryHistoryStore&&) noexcept = default;
	RobustDirectoryHistoryStore(const RobustDirectoryHistoryStore&) = default;

	RobustDirectoryHistoryStore& operator=(RobustDirectoryHistoryStore) noexcept;

	/*! \brief This function still trusts that the given directory still exists*/
	void OnNewDirectory(const FileSystem::Directory&);

	void SwitchToPrevious();
	void SwitchToNext();

	const FileSystem::Directory GetCurrent() const;

	friend void swap(RobustDirectoryHistoryStore&, RobustDirectoryHistoryStore&) noexcept;

private:
	DirectoryHistoryStore m_store;
};