#include "FilterHookedLineEdit.hpp"

#include <QKeyEvent>

void FilterHookedLineEdit::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape)
        emit escapePressed();
    QLineEdit::keyPressEvent(event);
}
