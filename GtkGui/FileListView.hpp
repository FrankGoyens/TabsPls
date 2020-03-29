#pragma once

#include <vector>
#include <string>
#include <memory>
#include <GtkGui/DragAndDrop.hpp>

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkListStore GtkListStore;

namespace FileListView
{
	struct ListWidgetWithStore
	{
		GtkWidget& listWidget;
		GtkListStore& store;
	};

	struct DragSourceListWidgetDataClosure
	{
		std::unique_ptr<DragAndDrop::GetDragData_Userdata> impl;
	};

	ListWidgetWithStore BuildFileListView();
	void FillListStoreWithFiles(GtkListStore& store, const std::vector<std::string>& fileNames);

	DragSourceListWidgetDataClosure ConnectDragSourceToListview(ListWidgetWithStore& listWidgetWithStore);
}
