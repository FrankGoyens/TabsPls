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

  const GtkTargetEntry target_list[] = {
      {"STRING", 0, TARGET_STRING},
      {"text/plain", 0, TARGET_STRING},
      {"text/uri-list", 0, TARGET_FILE}
    };
}

static gboolean drag_motion(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint t, gpointer user_data)
{
  g_print("Motion recieved!\n");

  return true;
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
  gtk_drag_dest_set(&widget, GTK_DEST_DEFAULT_ALL, &target_list[0], 3, GDK_ACTION_COPY);

  g_signal_connect(&widget, "drag-data-received", G_CALLBACK(receive_drag_data), NULL);
  g_signal_connect(&widget, "drag-drop", G_CALLBACK(drop), NULL);
  g_signal_connect(&widget, "drag-motion", G_CALLBACK(drag_motion), NULL);

  g_print("Drag set up\n");
}

static void print_hello (GtkWidget* widget, gpointer data)
{
  g_print ("Hello World\n");
}

static void activate(GtkApplication* app, gpointer user_data)
{
  GtkWidget* window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  GtkWidget* button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (window), button_box);

  GtkWidget* button = gtk_button_new_with_label ("Hello World");
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add (GTK_CONTAINER (button_box), button);

  make_widget_drop_target(*button_box);
  make_widget_drop_target(*button);
  make_widget_drop_target(*window);

  gtk_widget_show_all (window);
}

int main(int argc, char** argv)
{
	auto app = SmartGtk::MakeObject(gtk_application_new, g_object_unref, "com.frankgoyens.tabspls", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app.get(), "activate", G_CALLBACK (activate), nullptr);
	return g_application_run (G_APPLICATION (app.get()), argc, argv);
}
