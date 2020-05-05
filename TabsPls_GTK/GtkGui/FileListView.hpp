#pragma once

#include <vector>
#include <string>
#include <memory>

#include <GtkGui/DragAndDrop.hpp>
#include <GtkGui/DirectoryNavigationField.hpp>

#include <Gui/DirectoryChangedAction.hpp>

#include <TabsPlsCore/FileSystem.hpp>
#include <CurrentDirectoryProvider.hpp>

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkListStore GtkListStore;

namespace DirectoryNavigationField
{
	struct DirectoryChangedAction;
}

namespace DirectoryHistoryButtons
{
	struct DirectoryChangedAction;
}

struct DirectoryHistory;

namespace FileListView
{
	/*! \brief This is used to exchange data internally, this should be kept in scope as long as the list widget is in scope*/
	struct InternalUserdata
	{
		virtual ~InternalUserdata() = default;
	};

	struct DirectoryChangedAction : Gui::DirectoryChangedAction {};

	struct ListWidgetWithStore
	{
		GtkWidget& listWidget;
		GtkListStore& store;

		void RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>&);

		std::unique_ptr<InternalUserdata> _internalUserdata;
	};

	struct DragSourceListWidgetDataClosure
	{
		std::unique_ptr<DragAndDrop::GetDragData_Userdata> impl;
	};

	ListWidgetWithStore BuildFileListView(const std::shared_ptr<CurrentDirectoryProvider>&, const std::shared_ptr<DirectoryHistory>&);
	void FillListStoreWithFiles(GtkListStore& store, const FileSystem::RawPathVector& fileNames);

	DragSourceListWidgetDataClosure ConnectDragSourceToListview(ListWidgetWithStore& listWidgetWithStore);

	std::unique_ptr<DirectoryNavigationField::DirectoryChangedAction> CreateDirectoryChangedCallback_DirectoryNavigationField(ListWidgetWithStore& widgetWithStore);
	std::unique_ptr<DirectoryHistoryButtons::DirectoryChangedAction> CreateDirectoryChangedCallback_DirHistoryButtons(ListWidgetWithStore& widgetWithStore);
}
