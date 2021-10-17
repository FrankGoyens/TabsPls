#include "FutureWatchDialog.hpp"

#include "FutureWatchDialogWithProgressBar.hpp"

FutureWatchDialog::FutureWatchDialog(QWidget* parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);

    m_progressBar = &FutureWatchDialogWithProgressBar::MakeSingleProgressBarFutureDialog(*this);

    ConnectFunctionToFutureFinish([this] { accept(); });
}

void FutureWatchDialog::ConnectProgressReporterFromAnotherThread(
    std::shared_ptr<QObjectProgressReport> progressReport) {
    if (m_progressReport)
        return;

    m_progressReport = std::move(progressReport);
    FutureWatchDialogWithProgressBar::ConnectProgressReportToProgressBar(*m_progressReport, *m_progressBar);
}
