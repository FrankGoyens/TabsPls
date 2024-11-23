#pragma once

#include <QObject>

class MouseWheelClickEventFilter : public QObject {
    Q_OBJECT

  public:
    MouseWheelClickEventFilter() = default;

  signals:
    void MouseWheelClicked();

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

  private:
    bool m_middleMouseDown = false;
};