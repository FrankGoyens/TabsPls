#pragma once

#include <vector>
#include <string>
#include <memory>

#include <GtkGui/DragAndDrop.hpp>
#include <GtkGui/DirectoryNavigationField.hpp>
#include <model/FileSystem.hpp>

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkListStore GtkListStore;

namespace FileListView
{
	/*! \brief This is used to exchange data internally, this should be kept in scope as long as the list widget is in scope*/
	struct InternalUserdata
	{
		virtual ~InternalUserdata() = default;
	};

	struct ListWidgetWithStore
	{
		GtkWidget& listWidget;
		GtkListStore& store;

		std::unique_ptr<InternalUserdata> _internalUserdata;
	};

	struct DragSourceListWidgetDataClosure
	{
		std::unique_ptr<DragAndDrop::GetDragData_Userdata> impl;
	};

	ListWidgetWithStore BuildFileListView();
	void FillListStoreWithFiles(GtkListStore& store, const FileSystem::RawPathVector& fileNames);

	DragSourceListWidgetDataClosure ConnectDragSourceToListview(ListWidgetWithStore& listWidgetWithStore);

	std::unique_ptr<DirectoryNavigationField::DirectoryChangedAction> CreateDirectoryChangedCallback(ListWidgetWithStore& widgetWithStore);
}