#include "FileListView.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

#include <SmartGtk.hpp>

namespace
{
	enum
	{
		FILENAME_COLUMN = 0,
		N_COLUMNS
	};
}

namespace FileListView
{

	ListWidgetWithStore BuildFileListView()
	{
		const auto store = SmartGtk::MakeObject(gtk_list_store_new, 
			[](auto* obj){g_object_unref(G_OBJECT(obj));}, 
			N_COLUMNS, G_TYPE_STRING);

		auto* tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store.get()));
		auto* renderer = gtk_cell_renderer_text_new();

		auto* column = gtk_tree_view_column_new_with_attributes(
			"Filename",
			renderer, 
			"text", FILENAME_COLUMN,
			NULL);
		
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
		
		return {*tree, *store};
	}

	void FillListStoreWithFiles(GtkListStore& store, const std::vector<std::string>& fileNames)
	{
		GtkTreeIter it;

		for(auto& fileName: fileNames)
		{
			gtk_list_store_append (&store, &it);
			
			gtk_list_store_set(&store,
				&it,
				FILENAME_COLUMN, fileName.c_str(),
				-1);
		}
	}
}
