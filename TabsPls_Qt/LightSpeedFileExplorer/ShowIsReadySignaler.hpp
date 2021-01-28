#pragma once

#include <QObject>

class ShowIsReadySignaler : public QObject {
    Q_OBJECT

  signals:
    void ShowIsReady();

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};