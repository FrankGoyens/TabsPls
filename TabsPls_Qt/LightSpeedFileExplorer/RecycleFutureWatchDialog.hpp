#pragma once

#include <QDialog>

#include <TabsPlsCore/Send2Trash.hpp>

#include "FutureWatchDialog.hpp"

class QObjectProgressReport;
class QProgressBar;

class RecycleFutureWatchDialog : public QDialog, public FutureWatchDialog<TabsPlsPython::Send2Trash::AggregatedResult> {
    Q_OBJECT

  public:
    RecycleFutureWatchDialog(QWidget* parent, const QString& title);

    void ConnectProgressReporterFromAnotherThread(std::shared_ptr<QObjectProgressReport>);

  private:
    std::shared_ptr<QObjectProgressReport> m_progressReport = nullptr;
    QProgressBar* m_progressBar = nullptr;
};
