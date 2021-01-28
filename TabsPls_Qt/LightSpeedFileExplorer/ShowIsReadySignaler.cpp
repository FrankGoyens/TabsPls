#include "ShowIsReadySignaler.hpp"

#include <QShowEvent>

bool ShowIsReadySignaler::eventFilter(QObject* obj, QEvent* event) {
    // emit ShowIsReady();
    if (dynamic_cast<QShowEvent*>(event)) {
        emit ShowIsReady();
        return true;
    }
    return QObject::eventFilter(obj, event);
}
