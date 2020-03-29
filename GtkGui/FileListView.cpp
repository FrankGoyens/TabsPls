#include "FileListView.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

#include <SmartGtk.hpp>
#include <GtkGui/DragAndDrop.hpp>

namespace
{
	enum
	{
		FILENAME_COLUMN = 0,
		N_COLUMNS
	};
}

namespace
{
	class DragSourceGetDataImpl : public DragAndDrop::GetDragData_Userdata
	{
	public:
		DragSourceGetDataImpl(GtkWidget& listview): m_listview(listview){}

		std::string GetData() const override
		{ 
			auto* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(&m_listview));

			GtkTreeIter it;
			GtkTreeModel* model;
			
			if (gtk_tree_selection_get_selected (selection, &model, &it))
        	{
				gchar* filename;
				gtk_tree_model_get(model, &it, FILENAME_COLUMN, &filename, -1);
				std::string filenameString(filename);
				g_free(filename);

				return filenameString;
			}

			return "";
		}

	private:
		GtkWidget& m_listview;
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
		auto* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

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

	DragSourceListWidgetDataClosure ConnectDragSourceToListview(ListWidgetWithStore& listWidgetWithStore)
	{
		DragSourceListWidgetDataClosure closure;
		closure.impl = std::make_unique<DragSourceGetDataImpl>(listWidgetWithStore.listWidget);
        DragAndDrop::MakeDragSource(listWidgetWithStore.listWidget, *closure.impl);

		return closure;
	}
}
