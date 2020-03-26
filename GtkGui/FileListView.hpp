#pragma once

#include <vector>
#include <string>

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkListStore GtkListStore;

namespace FileListView
{
	struct ListWidgetWithStore
	{
		GtkWidget& listWidget;
		GtkListStore& store;
	};

	ListWidgetWithStore BuildFileListView();
	void FillListStoreWithFiles(GtkListStore& store, const std::vector<std::string>& fileNames);
}
