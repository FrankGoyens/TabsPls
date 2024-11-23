#include "MouseWheelClickEventFilter.hpp"

#include <QMouseEvent>

bool MouseWheelClickEventFilter::eventFilter(QObject* obj, QEvent* event) {
    if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        if (mouseEvent->buttons() & Qt::MiddleButton) {
            m_middleMouseDown = true;
        } else if (m_middleMouseDown) {
            m_middleMouseDown = false;
            emit MouseWheelClicked();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}
