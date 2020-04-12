#include "DirectoryHistoryButtons.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

#include <stack>

#include <FileSystemDirectory.hpp>

namespace
{
	struct DirectoryHistoryButtonsUserdata : DirectoryHistoryButtons::InternalUserdata
	{
		std::stack<FileSystem::Directory> previousDirs;
		std::stack<FileSystem::Directory> nextDirs;
	};
}

namespace DirectoryHistoryButtons
{
	DirectoryHistoryButtonWidget BuildDirectoryHistoryButtons()
	{
		auto userdata = std::make_unique<DirectoryHistoryButtonsUserdata>();

		auto* backButton = gtk_button_new_with_label(u8"←");
		auto* forwardButton = gtk_button_new_with_label(u8"→");

		auto* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

		gtk_container_add(GTK_CONTAINER(hbox), backButton);
		gtk_container_add(GTK_CONTAINER(hbox), forwardButton);

		DirectoryHistoryButtonWidget result = { *hbox, std::move(userdata) };

		return result;
	}
}