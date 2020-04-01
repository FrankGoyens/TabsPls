#include "FileListView.hpp"

extern "C"
{
#include <gtk/gtk.h>
}

#include <sstream>

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
	struct ListButtonPress_Userdata : FileListView::InternalUserdata
	{
		ListButtonPress_Userdata() : newItemWasAlreadySelected(false){}

		bool newItemWasAlreadySelected;
	};
}

gboolean view_selection_func (GtkTreeSelection *selection,
					GtkTreeModel     *model,
					GtkTreePath      *path,
					gboolean          path_currently_selected,
					gpointer          userdata)
{
	auto& dataFromPressEvent = *static_cast<ListButtonPress_Userdata*>(userdata);

	return !dataFromPressEvent.newItemWasAlreadySelected;
}

static void gtk_selection_foreach(GtkTreeModel* model,
                                GtkTreePath*,
                                GtkTreeIter* it,
                                gpointer data)
{
	auto& filenames = *static_cast<std::stringstream*>(data);

	gchar* filename;
	gtk_tree_model_get(model, it, FILENAME_COLUMN, &filename, -1);
	std::string filenameString(filename);
	g_free(filename);

	filenames << filenameString << std::endl;
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
			gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

			std::stringstream filenames;

			gtk_tree_selection_selected_foreach(selection, gtk_selection_foreach, static_cast<void*>(&filenames));

			return filenames.str();
		}

	private:
		GtkWidget& m_listview;
	};
}

gboolean list_button_release(GtkWidget *treeview,
               GdkEventButton  *event,
               gpointer   userdata)
{
	auto& castUserdata = *static_cast<ListButtonPress_Userdata*>(userdata);
	
	if(!castUserdata.newItemWasAlreadySelected)
		return FALSE;

	castUserdata.newItemWasAlreadySelected = false;

	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	GtkTreePath *path;

	if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
											(gint) event->x, 
											(gint) event->y,
											&path, NULL, NULL, NULL))
	{
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_path(selection, path);
		gtk_tree_path_free(path);
	}
	
	return FALSE;
}

gboolean list_button_press(GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
	auto& castUserdata = *static_cast<ListButtonPress_Userdata*>(userdata);
	castUserdata.newItemWasAlreadySelected = false;

	/* single click with the right mouse button? */
	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 1)
	{
		GtkTreeSelection *selection;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

		GtkTreePath *path;

		/* Get tree path for row that was clicked */
		if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
											(gint) event->x, 
											(gint) event->y,
											&path, NULL, NULL, NULL))
		{
			/*If the path was already selected, then prevent changing the selection
			to enable the user to drag the current selection if wanted*/
			castUserdata.newItemWasAlreadySelected = gtk_tree_selection_path_is_selected(selection, path);
			gtk_tree_path_free(path);
		}
	}

	return FALSE; /* we did not handle this */
}

namespace FileListView
{

	ListWidgetWithStore BuildFileListView()
	{
		const auto store = SmartGtk::MakeObject(gtk_list_store_new, 
			[](auto* obj){g_object_unref(G_OBJECT(obj));}, 
			N_COLUMNS, G_TYPE_STRING);

		auto* tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store.get()));
		auto internalUserData = std::make_unique<ListButtonPress_Userdata>();

		g_signal_connect(tree, "button-release-event", G_CALLBACK(list_button_release), static_cast<void*>(internalUserData.get()));
		g_signal_connect(tree, "button-press-event", G_CALLBACK(list_button_press), static_cast<void*>(internalUserData.get()));

		auto* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
		
		gtk_tree_selection_set_select_function(selection, view_selection_func, static_cast<void*>(internalUserData.get()), NULL);

		auto* renderer = gtk_cell_renderer_text_new();

		auto* column = gtk_tree_view_column_new_with_attributes(
			"Filename",
			renderer, 
			"text", FILENAME_COLUMN,
			NULL);
		
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
		

		ListWidgetWithStore result = {*tree, *store, nullptr};
		result._internalUserdata = std::move(internalUserData);
		return result;
	}

	void FillListStoreWithFiles(GtkListStore& store, const FileSystem::RawPathVector& fileNames)
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
