#include "DirectoryNavigationField.hpp"

#include <FileSystemDirectory.hpp>

#include <gtk/gtk.h>

namespace DirectoryNavigationField
{
	GtkWidget* BuildDirectoryNavigationField(const FileSystem::Directory& dir)
	{
		auto* const directoryEntryBuffer = gtk_entry_buffer_new(
			dir.path().c_str(),
			static_cast<gint>(dir.path().size()));

		auto* const directoryEntry = gtk_entry_new_with_buffer(directoryEntryBuffer);

		return directoryEntry;
	}
}