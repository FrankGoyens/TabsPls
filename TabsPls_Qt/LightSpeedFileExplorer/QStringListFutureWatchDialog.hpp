#pragma once

#include <QDialog>
#include <QFutureWatcher>

#include "FutureWatchDialog.hpp"

class QProgressBar;
class QObjectProgressReport;

class QStringListFutureWatchDialog : public QDialog, public FutureWatchDialog<QStringList> {
    Q_OBJECT
  public:
    QStringListFutureWatchDialog(QWidget* parent, const QString& title);

    void ConnectProgressReporterFromAnotherThread(std::shared_ptr<QObjectProgressReport>);

  private:
    std::shared_ptr<QObjectProgressReport> m_progressReport = nullptr;
    QProgressBar* m_progressBar = nullptr;
};