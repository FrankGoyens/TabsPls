#pragma once

#include <QDialog>

#include "FutureWatchDialog.hpp"

class QProgressBar;
class QObjectProgressReport;

class VoidFutureWatchDialog : public QDialog, public FutureWatchDialog<void> {
    Q_OBJECT
  public:
    VoidFutureWatchDialog(QWidget* parent, const QString& title);

    void ConnectProgressReporterFromAnotherThread(std::shared_ptr<QObjectProgressReport>);

  private:
    std::shared_ptr<QObjectProgressReport> m_progressReport = nullptr;
    QProgressBar* m_progressBar = nullptr;
};