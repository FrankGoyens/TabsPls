#include "DirectoryHistoryButtons.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

#include <stack>

#include <GtkGui/FileListView.hpp>
#include <GtkGui/DirectoryNavigationField.hpp>

#include <FileSystemDirectory.hpp>
#include <CurrentDirectoryProvider.hpp>

namespace
{
	struct DirectoryHistoryButtonsUserdata : DirectoryHistoryButtons::InternalUserdata
	{
		std::stack<FileSystem::Directory> previousDirs;
		std::stack<FileSystem::Directory> nextDirs;

		std::vector<std::weak_ptr<DirectoryHistoryButtons::DirectoryChangedAction>> directoryChangedActions;
	};
}

static void BackButtonClicked(GtkButton* button, gpointer userdata)
{
	auto& typedUserdata = *static_cast<DirectoryHistoryButtonsUserdata*>(userdata);
	
	if (typedUserdata.previousDirs.size() <= 1)
		return;

	typedUserdata.nextDirs.push(typedUserdata.previousDirs.top());
	typedUserdata.previousDirs.pop();

	Gui::DoActions(typedUserdata.directoryChangedActions, typedUserdata.previousDirs.top());
}

static void ForwardButtonClicked(GtkButton* button, gpointer userdata)
{
	auto& typedUserdata = *static_cast<DirectoryHistoryButtonsUserdata*>(userdata);

	if (typedUserdata.nextDirs.empty())
		return;

	typedUserdata.previousDirs.push(typedUserdata.nextDirs.top());
	typedUserdata.nextDirs.pop();

	Gui::DoActions(typedUserdata.directoryChangedActions, typedUserdata.previousDirs.top());
}

namespace DirectoryHistoryButtons
{
	DirectoryHistoryButtonWidget BuildDirectoryHistoryButtons(const CurrentDirectoryProvider& currentDirProdider)
	{
		auto userdata = std::make_unique<DirectoryHistoryButtonsUserdata>();

		userdata->previousDirs.push(currentDirProdider.Get());

		auto* backButton = gtk_button_new_with_label(u8"←");
		auto* forwardButton = gtk_button_new_with_label(u8"→");

		g_signal_connect(backButton, "clicked", G_CALLBACK(BackButtonClicked), userdata.get());
		g_signal_connect(forwardButton, "clicked", G_CALLBACK(ForwardButtonClicked), userdata.get());

		auto* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

		gtk_container_add(GTK_CONTAINER(hbox), backButton);
		gtk_container_add(GTK_CONTAINER(hbox), forwardButton);

		DirectoryHistoryButtonWidget result = { *hbox, std::move(userdata) };

		return result;
	}
	
	void DirectoryHistoryButtonWidget::RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>& action)
	{
		auto& typedUserdata = *static_cast<DirectoryHistoryButtonsUserdata*>(_internalUserdata.get());

		typedUserdata.directoryChangedActions.push_back(action);
	}

	namespace
	{
		template<typename BaseT>
		struct DirectoryChangedActionImpl : BaseT
		{
			DirectoryChangedActionImpl(DirectoryHistoryButtonsUserdata& userdata_) : userdata(userdata_) {}

			void Do(const FileSystem::Directory& dir) override
			{
				userdata.previousDirs.push(dir);
				
				userdata.nextDirs = std::stack<FileSystem::Directory>();
			}

			DirectoryHistoryButtonsUserdata& userdata;
		};
	}

	std::unique_ptr<FileListView::DirectoryChangedAction> CreateDirectoryChangedCallback_FileListView(DirectoryHistoryButtonWidget& widgetAndData)
	{
		auto& userdata = *static_cast<DirectoryHistoryButtonsUserdata*>(widgetAndData._internalUserdata.get());
		return std::make_unique<DirectoryChangedActionImpl<FileListView::DirectoryChangedAction>>(userdata);
	}

	std::unique_ptr<DirectoryNavigationField::DirectoryChangedAction> CreateDirectoryChangedCallback_DirectoryNavigationField(DirectoryHistoryButtonWidget& widgetAndData)
	{
		auto& userdata = *static_cast<DirectoryHistoryButtonsUserdata*>(widgetAndData._internalUserdata.get());
		return std::make_unique<DirectoryChangedActionImpl<DirectoryNavigationField::DirectoryChangedAction>>(userdata);
	}
}