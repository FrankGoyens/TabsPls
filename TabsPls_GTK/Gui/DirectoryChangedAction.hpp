#pragma once

namespace FileSystem
{
	class Directory;
}

namespace Gui
{
	struct DirectoryChangedAction
	{
		virtual ~DirectoryChangedAction() = default;
		virtual void Do(const FileSystem::Directory& dir) = 0;
	};

	template<typename ActionWeakPtrVecT, typename... Args>
	void DoActions(const ActionWeakPtrVecT& actions, Args&&... args)
	{
		for (auto& actionWeakPtr : actions)
			if (auto action = actionWeakPtr.lock())
				action->Do(std::forward<Args>(args)...);
	}
}