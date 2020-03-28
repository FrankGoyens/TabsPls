#pragma once

#include <string>

typedef struct _GtkWidget GtkWidget;

namespace DragAndDrop
{
    struct GetDragData_Userdata
    {
        virtual std::string GetData() const = 0;
    };

    void MakeDragSource(GtkWidget& widget, const GetDragData_Userdata& userdata);
    void MakeDropTarget(GtkWidget& widget);
}