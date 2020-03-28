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
#include <model/FileSystem.hpp>

/******************************************************************************/
int main (int argc, char **argv)
{
        guint           win_xsize       = 450;
        guint           win_ysize       = 200;

        /* Always start GTK+ first! */
        gtk_init (&argc, &argv);

        std::optional<std::string> directoryFromArgument;

        if(argc > 1 && FileSystem::IsDirectory(argv[1]))
                directoryFromArgument = argv[1];

        /* Create the widgets */
        auto* window  = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        auto* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        auto* well_dest = gtk_label_new("[drop here]");

	auto listviewWithStore = FileListView::BuildFileListView();
        /* Pack the widgets */
        gtk_container_add (GTK_CONTAINER (window), hbox);
        gtk_container_add (GTK_CONTAINER (hbox), &listviewWithStore.listWidget);
        gtk_container_add (GTK_CONTAINER (hbox), well_dest);

        /* Make the window big enough for some DnD action */
        gtk_window_set_default_size (GTK_WINDOW(window), win_xsize, win_ysize);

        const auto files = directoryFromArgument ? FileSystem::GetFilesInDirectory(*directoryFromArgument) : FileSystem::GetFilesInCurrentDirectory();

        FileListView::FillListStoreWithFiles(listviewWithStore.store, files);

        /* Connect the signals */
        g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

        struct DragSourceGetDataImpl : DragAndDrop::GetDragData_Userdata
        {
                std::string GetData() const override { return "This is data defined in main!";}
        };

        DragAndDrop::MakeDragSource(listviewWithStore.listWidget, DragSourceGetDataImpl());
        DragAndDrop::MakeDropTarget(*well_dest);

        /* Show the widgets */
        gtk_widget_show_all (window);

        /* Start the even loop */
        gtk_main ();

        return 0;
}
