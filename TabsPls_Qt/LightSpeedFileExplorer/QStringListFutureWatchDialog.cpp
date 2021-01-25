#include "QStringListFutureWatchDialog.hpp"

#include "FutureWatchDialogWithProgressBar.hpp"

QStringListFutureWatchDialog::QStringListFutureWatchDialog(QWidget* parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);

    m_progressBar = &FutureWatchDialogWithProgressBar::MakeSingleProgressBarFutureDialog(*this);

    ConnectFunctionToFutureFinish([this] { accept(); });
}

void QStringListFutureWatchDialog::ConnectProgressReporterFromAnotherThread(
    std::shared_ptr<QObjectProgressReport> progressReport) {
    if (m_progressReport)
        return;

    m_progressReport = std::move(progressReport);
    FutureWatchDialogWithProgressBar::ConnectProgressReportToProgressBar(*m_progressReport, *m_progressBar);
}