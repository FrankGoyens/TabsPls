#include "FutureWatchDialog.hpp"

#include "QObjectProgressReport.hpp"

static void ConnectProgressReportToProgressBar(const QObjectProgressReport& reporter, QProgressBar& progressBar) {
    QObject::connect(&reporter, &QObjectProgressReport::Updated, &progressBar, &QProgressBar::setValue,
                     Qt::ConnectionType::QueuedConnection);
    QObject::connect(&reporter, &QObjectProgressReport::MinimumSet, &progressBar, &QProgressBar::setMinimum,
                     Qt::ConnectionType::QueuedConnection);
    QObject::connect(&reporter, &QObjectProgressReport::MaximumSet, &progressBar, &QProgressBar::setMaximum,
                     Qt::ConnectionType::QueuedConnection);
}

FutureWatchDialog::FutureWatchDialog(QWidget* parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);

    m_progressBar = new QProgressBar();

    auto* rootLayout = new QVBoxLayout();
    rootLayout->addWidget(m_progressBar);

    setLayout(rootLayout);

    adjustSize();

    connect(&m_watcher, &QFutureWatcher<TabsPlsPython::Send2Trash::AggregatedResult>::finished, [this] { accept(); });
}

void FutureWatchDialog::SetFuture(const QFuture<TabsPlsPython::Send2Trash::AggregatedResult>& future) {
    m_watcher.setFuture(future);
}

TabsPlsPython::Send2Trash::AggregatedResult FutureWatchDialog::Result() const { return m_watcher.future().result(); }

void FutureWatchDialog::ConnectProgressReporterFromAnotherThread(
    std::shared_ptr<QObjectProgressReport> progressReport) {
    if (m_progressReport)
        return;

    m_progressReport = std::move(progressReport);
    ConnectProgressReportToProgressBar(*m_progressReport, *m_progressBar);
}
