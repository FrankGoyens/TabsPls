#include "FileListView.hpp"

extern "C"
{
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
}

#include <sstream>

#include <SmartGtk.hpp>
#include <GtkGui/DragAndDrop.hpp>
#include <FileSystemDirectory.hpp>

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
	struct ListWidgetWithStoreUserdata : FileListView::InternalUserdata
	{
		ListWidgetWithStoreUserdata(std::shared_ptr<CurrentDirectoryProvider> currentDirectoryProvider_,
			std::shared_ptr<DirectoryHistory> directoryHistory_) : 
			newItemWasAlreadySelected(false),
			currentDirectoryProvider(std::move(currentDirectoryProvider_)),
			directoryHistory(std::move(directoryHistory_))
		{}

		void DoActions(const FileSystem::Directory& dir) const
		{
			Gui::DoActions(directoryChangedActions, dir);
		}

		bool newItemWasAlreadySelected;
		std::vector<std::weak_ptr<FileListView::DirectoryChangedAction>> directoryChangedActions;
		std::shared_ptr<CurrentDirectoryProvider> currentDirectoryProvider;
		std::shared_ptr<DirectoryHistory> directoryHistory;
	};
}

static gboolean view_selection_func (GtkTreeSelection *selection,
					GtkTreeModel     *model,
					GtkTreePath      *path,
					gboolean          path_currently_selected,
					gpointer          userdata)
{
	auto& dataFromPressEvent = *static_cast<ListWidgetWithStoreUserdata*>(userdata);

	return !dataFromPressEvent.newItemWasAlreadySelected;
}

static std::string GetFilenameFromRow(GtkTreeModel* model, GtkTreeIter* it)
{
	gchar* filename;
	gtk_tree_model_get(model, it, FILENAME_COLUMN, &filename, -1);
	const std::string filenameString(filename);
	g_free(filename);

	return filenameString;
}

static void gtk_selection_foreach(GtkTreeModel* model,
                                GtkTreePath*,
                                GtkTreeIter* it,
                                gpointer data)
{
	auto& filenames = *static_cast<std::stringstream*>(data);

	const auto filenameString = GetFilenameFromRow(model, it);

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

static gboolean list_button_release(GtkWidget *treeview,
               GdkEventButton  *event,
               gpointer   userdata)
{
	auto& castUserdata = *static_cast<ListWidgetWithStoreUserdata*>(userdata);
	
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

static gboolean list_button_press(GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
	auto& castUserdata = *static_cast<ListWidgetWithStoreUserdata*>(userdata);
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

static void ResetFileListViewContent(GtkTreeView& view, const ListWidgetWithStoreUserdata& userdata, const FileSystem::Directory& dir)
{
	auto* model = gtk_tree_view_get_model(&view);

	gtk_list_store_clear(GTK_LIST_STORE(model));
	FileListView::FillListStoreWithFiles(*GTK_LIST_STORE(model), FileSystem::GetFilesInDirectory(dir));

	userdata.DoActions(dir);
}

static void ActivateRow(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column,	gpointer user_data)
{
	auto& typedUserdata = *static_cast<ListWidgetWithStoreUserdata*>(user_data);
	auto* model = gtk_tree_view_get_model(tree_view);
	
	GtkTreeIter it;
	if (gtk_tree_model_get_iter(model, &it, path))
		if (auto dir = FileSystem::Directory::FromPath(GetFilenameFromRow(model, &it)))
		{
			if (dir->path() == "..")
				*dir = typedUserdata.currentDirectoryProvider->Get().Parent();

			ResetFileListViewContent(*tree_view, typedUserdata, *dir);
		}
}

static void AddParentDirectoryToStore(GtkListStore& store)
{
	GtkTreeIter it;
	gtk_list_store_append(&store, &it);

	gtk_list_store_set(&store,
		&it,
		FILENAME_COLUMN, FileSystem::RawPath("..").c_str(),
		-1);
}

static bool GotoNextDirectoryKeysPressed(GdkEvent& event)
{
	const GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();
	return event.key.keyval == GDK_KEY_Right
		&& (event.key.state & modifiers) == GDK_MOD1_MASK;
}

static bool GotoPreviousDirectoryKeysPressed(GdkEvent& event)
{
	const GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();
	return event.key.keyval == GDK_KEY_Left
		&& (event.key.state & modifiers) == GDK_MOD1_MASK;
}

static bool GotoParentDirectoryKeysPressed(GdkEvent& event)
{
	const GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();
	return event.key.keyval == GDK_KEY_Up
		&& (event.key.state & modifiers) == GDK_MOD1_MASK;
}

static gboolean ListViewKeyPress(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{

	if (GotoParentDirectoryKeysPressed(*event))
	{
		auto& typedUserdata = *static_cast<ListWidgetWithStoreUserdata*>(user_data);
		
		ResetFileListViewContent(*GTK_TREE_VIEW(widget), typedUserdata, typedUserdata.currentDirectoryProvider->Get().Parent());

		return TRUE;
	}
	else if (GotoPreviousDirectoryKeysPressed(*event))
	{
		auto& typedUserdata = *static_cast<ListWidgetWithStoreUserdata*>(user_data);
		
		typedUserdata.directoryHistory->RequestPreviousDirectory();
	
		return TRUE;
	}
	else if (GotoNextDirectoryKeysPressed(*event))
	{
		auto& typedUserdata = *static_cast<ListWidgetWithStoreUserdata*>(user_data);

		typedUserdata.directoryHistory->RequestNextDirectory();

		return TRUE;
	}

	return FALSE;
}

static gboolean ListViewKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	return FALSE;
}

static void SetupKeyEventHandling(GtkTreeView& tree, ListWidgetWithStoreUserdata& internalUserdata)
{
	g_signal_connect(&tree, "key-press-event", G_CALLBACK(ListViewKeyPress), static_cast<void*>(&internalUserdata));
	g_signal_connect(&tree, "key-release-event", G_CALLBACK(ListViewKeyRelease), static_cast<void*>(&internalUserdata));
}

namespace FileListView
{

	ListWidgetWithStore BuildFileListView(const std::shared_ptr<CurrentDirectoryProvider>& currentDirProvider, const std::shared_ptr<DirectoryHistory>& directoryHistory)
	{
		const auto store = SmartGtk::MakeObject(gtk_list_store_new, 
			[](auto* obj){g_object_unref(G_OBJECT(obj));}, 
			N_COLUMNS, G_TYPE_STRING);

		auto* tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store.get()));
		auto internalUserData = std::make_unique<ListWidgetWithStoreUserdata>(currentDirProvider, directoryHistory);

		g_signal_connect(tree, "button-release-event", G_CALLBACK(list_button_release), static_cast<void*>(internalUserData.get()));
		g_signal_connect(tree, "button-press-event", G_CALLBACK(list_button_press), static_cast<void*>(internalUserData.get()));
		g_signal_connect(tree, "row-activated", G_CALLBACK(ActivateRow), static_cast<void*>(internalUserData.get()));

		SetupKeyEventHandling(*GTK_TREE_VIEW(tree), *internalUserData);

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

		AddParentDirectoryToStore(store);

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
	
	namespace
	{
		template<typename BaseT>
		struct DirectoryChangedActionImpl : BaseT
		{
			DirectoryChangedActionImpl(GtkListStore& store_) : store(store_) {}

			void Do(const FileSystem::Directory& dir) override
			{
				gtk_list_store_clear(&store);
				FillListStoreWithFiles(store, FileSystem::GetFilesInDirectory(dir));
			}

			GtkListStore& store;
		};
	}

	std::unique_ptr<DirectoryNavigationField::DirectoryChangedAction> CreateDirectoryChangedCallback_DirectoryNavigationField(ListWidgetWithStore& widgetWithStore)
	{
		return std::make_unique<DirectoryChangedActionImpl<DirectoryNavigationField::DirectoryChangedAction>>(widgetWithStore.store);
	}

	std::unique_ptr<DirectoryHistoryButtons::DirectoryChangedAction> CreateDirectoryChangedCallback_DirHistoryButtons(ListWidgetWithStore& widgetWithStore)
	{
		return std::make_unique<DirectoryChangedActionImpl<DirectoryHistoryButtons::DirectoryChangedAction>>(widgetWithStore.store);
	}
	
	void ListWidgetWithStore::RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>& action)
	{
		auto& typedUserdata = *static_cast<ListWidgetWithStoreUserdata*>(_internalUserdata.get());

		typedUserdata.directoryChangedActions.push_back(action);
	}
}
