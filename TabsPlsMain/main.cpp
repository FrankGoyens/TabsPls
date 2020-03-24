extern "C"
{
#include <gtk/gtk.h>
}

#include <iostream>

#include <SmartGtk.hpp>

namespace
{
  enum
  {
    TARGET_STRING,
    TARGET_FILE
  };

  constexpr unsigned int _BYTE = 8;

  const GtkTargetEntry target_list[] = {
    {"text/plain", 0, TARGET_STRING},
    {"text/uri-list", 0, TARGET_FILE}
  };

  constexpr guint n_targets = 2;
}

static void drag_leave(GtkWidget *widget, GdkDragContext *context, guint time, gpointer user_data)
{
  g_print("Drag leave!\n");
}

static gboolean drag_motion(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint t, gpointer user_data)
{
  g_print("Motion recieved!\n");

  return false;
}

static gboolean drop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data)
{
  g_print("Drop recieved!\n");

  return true;
}

static void receive_drag_data(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint target_type, guint time, gpointer user_data)
{
  g_print("Data received!\n");
}

static void make_widget_drop_target(GtkWidget& widget)
{
  gtk_drag_dest_set(&widget, GTK_DEST_DEFAULT_ALL, &target_list[0], n_targets, GDK_ACTION_COPY);

  g_signal_connect(&widget, "drag-data-received", G_CALLBACK(receive_drag_data), NULL);
  g_signal_connect(&widget, "drag-drop", G_CALLBACK(drop), NULL);
  g_signal_connect(&widget, "drag-motion", G_CALLBACK(drag_motion), NULL);
  g_signal_connect(&widget, "drag-leave", G_CALLBACK(drag_leave), NULL);

  g_print("Drop set up\n");
}

static void drag_data_get_handl(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint target_type, guint time, gpointer user_data)
{
  const gchar *name = gtk_widget_get_name (widget);
  const gchar *string_data = "This is data from the source.";
  const glong integer_data = 42;

  g_print ("%s: drag_data_get_handl\n", name);
  g_assert (selection_data != NULL);

  g_print (" Sending ");
  switch (target_type)
  {
          /* case TARGET_SOME_OBJECT:
            * Serialize the object and send as a string of bytes.
            * Pixbufs, (UTF-8) text, and URIs have their own convenience
            * setter functions */

  case TARGET_STRING:
          g_print ("string: %s", string_data);
          gtk_selection_data_set
          (
                  selection_data,
                  gtk_selection_data_get_target(selection_data),
                  _BYTE,
                  (guchar*) string_data,
                  strlen (string_data)
          );
          break;

  default:
          /* Default to some a safe target instead of fail. */
          g_assert_not_reached ();
  }

  g_print (".\n");
}


static void drag_begin_handl(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
  const gchar *name = gtk_widget_get_name (widget);
  g_print ("%s: drag_begin_handl\n", name);
}

static void drag_end_handl(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
  const gchar *name = gtk_widget_get_name (widget);
  g_print ("%s: drag_end_handl\n", name);
}

static void drag_data_delete_handl(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
  // We aren't moving or deleting anything here
  const gchar *name = gtk_widget_get_name (widget);
  g_print ("%s: drag_data_delete_handl\n", name);
}

static void make_widget_source_drag_target(GtkWidget& widget)
{
  gtk_drag_source_set(
    &widget,            /* widget will be drag-able */
    GDK_BUTTON1_MASK,       /* modifier that will start a drag */
    target_list,            /* lists of target to support */
    n_targets,              /* size of list */
    GDK_ACTION_COPY         /* what to do with data after dropped */);

    
  /* All possible source signals */
  g_signal_connect (&widget, "drag-data-get",
          G_CALLBACK (drag_data_get_handl), NULL);

  g_signal_connect (&widget, "drag-data-delete",
          G_CALLBACK (drag_data_delete_handl), NULL);

  g_signal_connect (&widget, "drag-begin",
          G_CALLBACK (drag_begin_handl), NULL);

  g_signal_connect (&widget, "drag-end",
          G_CALLBACK (drag_end_handl), NULL);

  g_print("Drag set up\n");
}

static void print_hello (GtkWidget* widget, gpointer data)
{
  g_print ("Hello World\n");
}

static void activate(GtkApplication* app, gpointer user_data)
{
  auto* window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  auto* button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  auto* button = gtk_button_new_with_label ("Hello World");

  auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,30);
  auto* drag_button = gtk_button_new_with_label("[Coins]");
  auto* drop_label = gtk_label_new("[Drop well]");
  
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_container_add (GTK_CONTAINER (button_box), button);
  gtk_container_add (GTK_CONTAINER (box), button_box);
  gtk_container_add (GTK_CONTAINER (box), drag_button);
  gtk_container_add (GTK_CONTAINER (box), drop_label);
  
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);

  make_widget_source_drag_target(*drag_button);
  make_widget_drop_target(*drop_label);

  gtk_widget_show_all (window);
}

int main(int argc, char** argv)
{
	auto app = SmartGtk::MakeObject(gtk_application_new, g_object_unref, "com.frankgoyens.tabspls", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app.get(), "activate", G_CALLBACK (activate), nullptr);
	return g_application_run (G_APPLICATION (app.get()), argc, argv);
}
