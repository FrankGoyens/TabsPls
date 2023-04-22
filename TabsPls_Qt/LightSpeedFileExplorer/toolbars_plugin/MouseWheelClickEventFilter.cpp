#include "MouseWheelClickEventFilter.hpp"

#include <QMouseEvent>

bool MouseWheelClickEventFilter::eventFilter(QObject* obj, QEvent* event) {
    if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        if (mouseEvent->buttons() & Qt::MiddleButton) {
            emit MouseWheelClicked();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}
