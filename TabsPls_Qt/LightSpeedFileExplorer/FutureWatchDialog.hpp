#pragma once

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressBar>
#include <QVBoxLayout>

#include <TabsPlsCore/Send2Trash.hpp>

class FutureWatchDialog : public QDialog {
    Q_OBJECT

  public:
    FutureWatchDialog(QWidget* parent, const QString& title);

    void SetFuture(const QFuture<TabsPlsPython::Send2Trash::AggregatedResult>&);
    TabsPlsPython::Send2Trash::AggregatedResult Result() const;

  private:
    QFutureWatcher<TabsPlsPython::Send2Trash::AggregatedResult> m_watcher;
};
