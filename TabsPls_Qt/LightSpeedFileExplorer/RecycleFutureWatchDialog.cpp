#include "RecycleFutureWatchDialog.hpp"

#include "FutureWatchDialogWithProgressBar.hpp"
#include "QObjectProgressReport.hpp"

RecycleFutureWatchDialog::RecycleFutureWatchDialog(QWidget* parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);

    m_progressBar = &FutureWatchDialogWithProgressBar::MakeSingleProgressBarFutureDialog(*this);

    ConnectFunctionToFutureFinish([this] { accept(); });
}

void RecycleFutureWatchDialog::ConnectProgressReporterFromAnotherThread(
    std::shared_ptr<QObjectProgressReport> progressReport) {
    if (m_progressReport)
        return;

    m_progressReport = std::move(progressReport);
    FutureWatchDialogWithProgressBar::ConnectProgressReportToProgressBar(*m_progressReport, *m_progressBar);
}