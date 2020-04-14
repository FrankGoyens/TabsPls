#include "DirectoryNavigationField.hpp"

#include <FileSystemDirectory.hpp>

#include <gtk/gtk.h>

namespace
{
	struct NavigationDirectoryFieldUserdata : DirectoryNavigationField::InternalUserdata
	{
		NavigationDirectoryFieldUserdata(FileSystem::Directory lastValidDirectory_) :
			lastValidDirectory(std::move(lastValidDirectory_))
		{}

		void DoActions(const FileSystem::Directory& dir)
		{
			Gui::DoActions(directoryChangedActions, dir);
		}

		FileSystem::Directory lastValidDirectory;
		std::vector<std::weak_ptr<DirectoryNavigationField::DirectoryChangedAction>> directoryChangedActions;
	};

	template<typename BaseT>
	struct DirectoryChangedActionImpl : BaseT
	{
		DirectoryChangedActionImpl(GtkEntry& entry_, NavigationDirectoryFieldUserdata& entryUserdata_) :
			entry(entry_),
			entryUserdata(entryUserdata_)
		{}

		void Do(const FileSystem::Directory& dir)
		{
			entryUserdata.lastValidDirectory = dir;
			gtk_entry_set_text(&entry, dir.path().c_str());
		}

		GtkEntry& entry;
		NavigationDirectoryFieldUserdata& entryUserdata;
	};
}

static void ActivateDirectoryEntry(GtkEntry* entry, gpointer userdata)
{
	auto& typedUserdata = *static_cast<NavigationDirectoryFieldUserdata*>(userdata);

	if (const auto newDirectory = FileSystem::Directory::FromPath(gtk_entry_get_text(entry)))
	{
		typedUserdata.lastValidDirectory = *newDirectory;
		
		gtk_entry_set_text(entry, newDirectory->path().c_str());

		typedUserdata.DoActions(*newDirectory);
	}
	else
		gtk_entry_set_text(entry, typedUserdata.lastValidDirectory.path().c_str());
}

namespace DirectoryNavigationField
{
	DirectoryNavigationFieldWidget BuildDirectoryNavigationField(const FileSystem::Directory& dir)
	{
		auto* const directoryEntryBuffer = gtk_entry_buffer_new(
			dir.path().c_str(),
			static_cast<gint>(dir.path().size()));

		auto* const directoryEntry = gtk_entry_new_with_buffer(directoryEntryBuffer);
		gtk_entry_set_width_chars(GTK_ENTRY(directoryEntry), 100);

		auto userdata = std::make_unique<NavigationDirectoryFieldUserdata>(dir);

		g_signal_connect(directoryEntry, "activate", G_CALLBACK(ActivateDirectoryEntry), static_cast<void*>(userdata.get()));

		return DirectoryNavigationFieldWidget(*directoryEntry, std::move(userdata));
	}

	template<typename BaseT>
	static std::unique_ptr<DirectoryChangedActionImpl<BaseT>> CreateDirectoryChangedCallback(DirectoryNavigationFieldWidget& widgetWithStore)
	{
		auto& typedUserData = *static_cast<NavigationDirectoryFieldUserdata*>(widgetWithStore._internalUserdata.get());

		return std::make_unique<DirectoryChangedActionImpl<BaseT>>(*GTK_ENTRY(&widgetWithStore.widget), typedUserData);
	}

	std::unique_ptr<FileListView::DirectoryChangedAction> CreateDirectoryChangedCallback_FileListView(DirectoryNavigationFieldWidget& widgetWithStore)
	{
		return CreateDirectoryChangedCallback<FileListView::DirectoryChangedAction>(widgetWithStore);
	}

	std::unique_ptr<DirectoryHistoryButtons::DirectoryChangedAction> CreateDirectoryChangedCallback_DirHistoryButtons(DirectoryNavigationFieldWidget& widgetWithStore)
	{
		return CreateDirectoryChangedCallback<DirectoryHistoryButtons::DirectoryChangedAction>(widgetWithStore);
	}
	
	void DirectoryNavigationFieldWidget::RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>& action)
	{
		auto& typedUserData = *static_cast<NavigationDirectoryFieldUserdata*>(_internalUserdata.get());
		typedUserData.directoryChangedActions.push_back(action);
	}
	
	const FileSystem::Directory& DirectoryNavigationFieldWidget::Get() const
	{
		auto& typedUserData = *static_cast<NavigationDirectoryFieldUserdata*>(_internalUserdata.get());

		return typedUserData.lastValidDirectory;
	}
}