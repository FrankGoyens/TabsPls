/* TestDnD - main.c : Simple tutorial for GTK+ Drag-N-Drop
 * Copyright (C) 2005 Ryan McDougall.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <cstring>

#include <string>
#include <optional>

#include <GtkGui/FileListView.hpp>
#include <GtkGui/DragAndDrop.hpp>
#include <GtkGui/DirectoryNavigationField.hpp>
#include <GtkGui/DirectoryHistoryButtons.hpp>

#include <model/FileSystem.hpp>
#include <FileSystemDirectory.hpp>

/******************************************************************************/
int main (int argc, char **argv)
{
        guint           win_xsize       = 450;
        guint           win_ysize       = 200;

        /* Always start GTK+ first! */
        gtk_init (&argc, &argv);

        std::optional<FileSystem::RawPath> directoryStringFromArgument;

        if (argc > 1)
            directoryStringFromArgument = FileSystem::RawPath(argv[1]);

        std::optional<FileSystem::Directory> directoryFromArgument;
        if(directoryStringFromArgument) 
            directoryFromArgument = FileSystem::Directory::FromPath(*directoryStringFromArgument);

        /* Create the widgets */
        auto* window  = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        auto* topLevelHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        auto* well_dest = gtk_label_new("[drop here]");

        auto* topLevelVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

        auto* navigationHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

        auto currentDirectory = directoryFromArgument ? *directoryFromArgument : FileSystem::Directory::FromCurrentWorkingDirectory();
        
        std::shared_ptr<DirectoryNavigationField::DirectoryNavigationFieldWidget> directoryEntry = 
            std::make_shared<DirectoryNavigationField::DirectoryNavigationFieldWidget>(
                DirectoryNavigationField::BuildDirectoryNavigationField(currentDirectory));

        auto listviewWithStore = FileListView::BuildFileListView(directoryEntry);
        const auto files = directoryFromArgument ? FileSystem::GetFilesInDirectory(*directoryFromArgument) : FileSystem::GetFilesInCurrentDirectory();
        FileListView::FillListStoreWithFiles(listviewWithStore.store, files);
            
        std::shared_ptr<DirectoryNavigationField::DirectoryChangedAction> directoryChangedActionFromNavField = FileListView::CreateDirectoryChangedCallback(listviewWithStore);
        std::shared_ptr<FileListView::DirectoryChangedAction> directoryChangedActionFromListView = DirectoryNavigationField::CreateDirectoryChangedCallback(*directoryEntry);

        directoryEntry->RegisterDirectoryChanged(directoryChangedActionFromNavField);
        listviewWithStore.RegisterDirectoryChanged(directoryChangedActionFromListView);

        auto directoryHistoryButtons = DirectoryHistoryButtons::BuildDirectoryHistoryButtons();

        /* Pack the widgets */
        gtk_container_add (GTK_CONTAINER (window), topLevelHbox);

        gtk_container_add (GTK_CONTAINER (navigationHbox), &directoryHistoryButtons.widget);
        gtk_container_add (GTK_CONTAINER (navigationHbox), &directoryEntry->widget);

        gtk_container_add (GTK_CONTAINER (topLevelVbox), navigationHbox);
        gtk_container_add (GTK_CONTAINER (topLevelVbox), &listviewWithStore.listWidget);

        gtk_container_add (GTK_CONTAINER (topLevelHbox), topLevelVbox);
        gtk_container_add (GTK_CONTAINER (topLevelHbox), well_dest);

        /* Make the window big enough for some DnD action */
        gtk_window_set_default_size (GTK_WINDOW(window), win_xsize, win_ysize);

        /* Connect the signals */
        g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

        const auto dragSourceDataClosure = FileListView::ConnectDragSourceToListview(listviewWithStore);
        DragAndDrop::MakeDropTarget(*well_dest);

        /* Show the widgets */
        gtk_widget_show_all (window);

        /* Start the even loop */
        gtk_main ();

        return 0;
}
