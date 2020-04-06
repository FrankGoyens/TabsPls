#include "DragAndDrop.hpp"

#include <gtk/gtk.h>

/******************************************************************************/
#define _BYTE   8
#define _WORD   16
#define _DWORD  32


/******************************************************************************/
/* Define a list of data types called "targets" that a destination widget will
 * accept. The string type is arbitrary, and negotiated between DnD widgets by
 * the developer. An enum or GQuark can serve as the integer target id. */
enum {
        TARGET_STRING
};

gchar STRING_string_id[] = "STRING";
gchar text_plain_string_id[] = "text/plain";
gchar text_uri_list_string_id[] = "text/uri-list";

/* datatype (string), restrictions on DnD (GtkTargetFlags), datatype (int) */
static GtkTargetEntry target_list[] = {
        { STRING_string_id,     0, TARGET_STRING },
        { text_plain_string_id, 0, TARGET_STRING },
        { text_uri_list_string_id, 0, TARGET_STRING } //This one is required for the file explorer on W10
};

static guint n_targets = G_N_ELEMENTS (target_list);


/******************************************************************************/
/* Signal receivable by destination */

/* Emitted when the data has been received from the source. It should check
 * the GtkSelectionData sent by the source, and do something with it. Finally
 * it needs to finish the operation by calling gtk_drag_finish, which will emit
 * the "data-delete" signal if told to. */
static void
drag_data_received_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
        GtkSelectionData *selection_data, guint target_type, guint time,
        gpointer data)
{
        gchar   *_sdata;

        gboolean dnd_success = FALSE;
        gboolean delete_selection_data = FALSE;

        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_data_received_handl\n", name);


        /* Deal with what we are given from source */
        if((selection_data != NULL) && (gtk_selection_data_get_length(selection_data) >= 0))
        {
                if (gdk_drag_context_get_suggested_action(context) == GDK_ACTION_ASK)
                {
                /* Ask the user to move or copy, then set the context action. */
                }

                if (gdk_drag_context_get_suggested_action(context) == GDK_ACTION_MOVE)
                        delete_selection_data = TRUE;

                /* Check that we got the format we can use */
                g_print (" Receiving ");
                switch (target_type)
                {
                        case TARGET_STRING:
                                _sdata = (gchar*)gtk_selection_data_get_data(selection_data);
                                g_print ("string: %s", _sdata);
                                dnd_success = TRUE;
                                break;

                        default:
                                g_print ("nothing good");
                }

                g_print (".\n");
        }

        if (dnd_success == FALSE)
        {
                g_print ("DnD data transfer failed!\n");
        }

        gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}

/* Emitted when a drag is over the destination */
static gboolean
drag_motion_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint t,
        gpointer user_data)
{
        // Fancy stuff here. This signal spams the console something horrible.
        //const gchar *name = gtk_widget_get_name (widget);
        //g_print ("%s: drag_motion_handl\n", name);
        return  FALSE;
}

/* Emitted when a drag leaves the destination */
static void
drag_leave_handl
(GtkWidget *widget, GdkDragContext *context, guint time, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_leave_handl\n", name);
}

/* Emitted when the user releases (drops) the selection. It should check that
 * the drop is over a valid part of the widget (if its a complex widget), and
 * itself to return true if the operation should continue. Next choose the
 * target type it wishes to ask the source for. Finally call gtk_drag_get_data
 * which will emit "drag-data-get" on the source. */
static gboolean
drag_drop_handl
(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time,
        gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_drop_handl\n", name);

    //We could do more advanced stuff here, but we only deal with file uri's and simple widgets right now
    return true; 
}


/******************************************************************************/
/* Signals receivable by source */

/* Emitted after "drag-data-received" is handled, and gtk_drag_finish is called
 * with the "delete" parameter set to TRUE (when DnD is GDK_ACTION_MOVE). */
static void
drag_data_delete_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
        // We aren't moving or deleting anything here
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_data_delete_handl\n", name);
}

/* Emitted when the destination requests data from the source via
 * gtk_drag_get_data. It should attempt to provide its data in the form
 * requested in the target_type passed to it from the destination. If it cannot,
 * it should default to a "safe" type such as a string or text, even if only to
 * print an error. Then use gtk_selection_data_set to put the source data into
 * the allocated selection_data object, which will then be passed to the
 * destination. This will cause "drag-data-received" to be emitted on the
 * destination. GdkSelectionData is based on X's selection mechanism which,
 * via X properties, is only capable of storing data in blocks of 8, 16, or
 * 32 bit units. */
static void
drag_data_get_handl
(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data,
        guint target_type, guint time, gpointer user_data)
{
    const auto* unmarshalledUserdata = static_cast<DragAndDrop::GetDragData_Userdata*>(user_data);

        const gchar *name = gtk_widget_get_name (widget);
        const auto string_data = unmarshalledUserdata == nullptr ? "": unmarshalledUserdata->GetData();
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
                g_print ("string: %s", string_data.c_str());
                gtk_selection_data_set
                (
                        selection_data,
                        gtk_selection_data_get_target(selection_data),
                        _BYTE,
                        (guchar*) string_data.c_str(),
                        static_cast<gint>(string_data.size())
                );
                break;

        default:
                /* Default to some a safe target instead of fail. */
                g_assert_not_reached ();
        }

        g_print (".\n");
}

/* Emitted when DnD begins. This is often used to present custom graphics. */
static void
drag_begin_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_begin_handl\n", name);
}

/* Emitted when DnD ends. This is used to clean up any leftover data. */
static void
drag_end_handl
(GtkWidget *widget, GdkDragContext *context, gpointer user_data)
{
        const gchar *name = gtk_widget_get_name (widget);
        g_print ("%s: drag_end_handl\n", name);
}

namespace DragAndDrop
{
    void MakeDragSource(GtkWidget& widget, const GetDragData_Userdata& userdata)
    {
        gtk_drag_source_set
        (
                &widget,            /* widget will be drag-able */
                GDK_BUTTON1_MASK,       /* modifier that will start a drag */
                target_list,            /* lists of target to support */
                n_targets,              /* size of list */
                GDK_ACTION_COPY         /* what to do with data after dropped */
        );

        /* All possible source signals */
        g_signal_connect (&widget, "drag-data-get",
                G_CALLBACK (drag_data_get_handl), const_cast<void*>(static_cast<const void*>(&userdata)));

        g_signal_connect (&widget, "drag-data-delete",
                G_CALLBACK (drag_data_delete_handl), NULL);

        g_signal_connect (&widget, "drag-begin",
                G_CALLBACK (drag_begin_handl), NULL);

        g_signal_connect (&widget, "drag-end",
                G_CALLBACK (drag_end_handl), NULL);
    }
    
    void MakeDropTarget(GtkWidget& widget)
    {
       /* Make the "well label" a DnD destination. */
       gtk_drag_dest_set
       (
               &widget,              /* widget that will accept a drop */
               GTK_DEST_DEFAULT_ALL, /* default actions for dest on DnD */
               target_list,            /* lists of target to support */
               n_targets,              /* size of list */
               GDK_ACTION_COPY         /* what to do with data after dropped */
       );

        /* All possible destination signals */
       g_signal_connect (&widget, "drag-data-received",
               G_CALLBACK(drag_data_received_handl), NULL);

       g_signal_connect (&widget, "drag-leave",
               G_CALLBACK (drag_leave_handl), NULL);

       g_signal_connect (&widget, "drag-motion",
               G_CALLBACK (drag_motion_handl), NULL);

       g_signal_connect (&widget, "drag-drop",
               G_CALLBACK (drag_drop_handl), NULL);
    }
}