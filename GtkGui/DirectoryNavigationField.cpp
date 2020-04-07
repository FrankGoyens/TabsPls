#include "DirectoryNavigationField.hpp"

#include <FileSystemDirectory.hpp>

#include <gtk/gtk.h>

namespace
{
	struct NavigationDirectoryFieldUserdata : DirectoryNavigationField::InternalUserdata
	{
		NavigationDirectoryFieldUserdata(FileSystem::Directory lastValidDirectory_, std::weak_ptr<DirectoryNavigationField::DirectoryChangedAction> directoryChangedAction_) :
			lastValidDirectory(std::move(lastValidDirectory_)),
			directoryChangedAction(std::move(directoryChangedAction_))
		{}

		FileSystem::Directory lastValidDirectory;
		std::weak_ptr<DirectoryNavigationField::DirectoryChangedAction> directoryChangedAction;
	};
}

static void ActivateDirectoryEntry(GtkEntry* entry, gpointer userdata)
{
	auto& typedUserdata = *static_cast<NavigationDirectoryFieldUserdata*>(userdata);

	if (const auto newDirectory = FileSystem::Directory::FromPath(gtk_entry_get_text(entry)))
	{
		typedUserdata.lastValidDirectory = *newDirectory;
		
		gtk_entry_set_text(entry, newDirectory->path().c_str());

		if (const auto validChangedCallback = typedUserdata.directoryChangedAction.lock())
			validChangedCallback->Do(*newDirectory);
	}
	else
		gtk_entry_set_text(entry, typedUserdata.lastValidDirectory.path().c_str());
}

namespace DirectoryNavigationField
{
	DirectoryNavigationFieldWidget BuildDirectoryNavigationField(const FileSystem::Directory& dir, const std::weak_ptr<DirectoryChangedAction>& directoryChangedAction)
	{
		auto* const directoryEntryBuffer = gtk_entry_buffer_new(
			dir.path().c_str(),
			static_cast<gint>(dir.path().size()));

		auto* const directoryEntry = gtk_entry_new_with_buffer(directoryEntryBuffer);

		auto userdata = std::make_unique<NavigationDirectoryFieldUserdata>(dir, directoryChangedAction);

		g_signal_connect(directoryEntry, "activate", G_CALLBACK(ActivateDirectoryEntry), static_cast<void*>(userdata.get()));

		DirectoryNavigationFieldWidget result = { *directoryEntry,  std::move(userdata) };

		return result;
	}
}