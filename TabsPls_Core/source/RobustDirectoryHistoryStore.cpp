#include <TabsPlsCore/RobustDirectoryHistoryStore.hpp>

RobustDirectoryHistoryStore& RobustDirectoryHistoryStore::operator=(RobustDirectoryHistoryStore other) noexcept
{
	swap(*this, other);
	return *this;
}

void RobustDirectoryHistoryStore::OnNewDirectory(const FileSystem::Directory& dir)
{
	m_store.OnNewDirectory(dir);
}

void RobustDirectoryHistoryStore::SwitchToPrevious()
{
	const auto current = GetCurrent();
	m_store.SwitchToPrevious();
	
	const auto previousExistingDir = FileSystem::Directory::FromPath(GetCurrent().path());
	if (!previousExistingDir)
	{
		m_store = DirectoryHistoryStore();
		m_store.OnNewDirectory(current);
	}
}

void RobustDirectoryHistoryStore::SwitchToNext()
{
	const auto current = GetCurrent();
	m_store.SwitchToNext();

	const auto nextExistingDir = FileSystem::Directory::FromPath(GetCurrent().path());
	if (!nextExistingDir)
	{
		m_store = DirectoryHistoryStore();
		m_store.OnNewDirectory(current);
	}
}

const FileSystem::Directory RobustDirectoryHistoryStore::GetCurrent() const
{
	return m_store.GetCurrent();
}

void swap(RobustDirectoryHistoryStore& first, RobustDirectoryHistoryStore& second) noexcept
{
	using std::swap;
	swap(first.m_store, second.m_store);
}
