#include "VoidFutureWatchDialog.hpp"

#include <QProgressBar>
#include <QVBoxLayout>

#include "FutureWatchDialogWithProgressBar.hpp"

VoidFutureWatchDialog::VoidFutureWatchDialog(QWidget* parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);

    m_progressBar = &FutureWatchDialogWithProgressBar::MakeSingleProgressBarFutureDialog(*this);

    ConnectFunctionToFutureFinish([this] { accept(); });
}

void VoidFutureWatchDialog::ConnectProgressReporterFromAnotherThread(
    std::shared_ptr<QObjectProgressReport> progressReport) {
    if (m_progressReport)
        return;

    m_progressReport = std::move(progressReport);
    FutureWatchDialogWithProgressBar::ConnectProgressReportToProgressBar(*m_progressReport, *m_progressBar);
}
