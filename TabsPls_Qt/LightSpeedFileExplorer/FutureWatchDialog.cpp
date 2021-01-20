#include "FutureWatchDialog.hpp"

FutureWatchDialog::FutureWatchDialog(QWidget* parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);

    auto* progressBar = new QProgressBar();

    auto* rootLayout = new QVBoxLayout();
    rootLayout->addWidget(progressBar);

    setLayout(rootLayout);

    adjustSize();

    connect(&m_watcher, &QFutureWatcher<TabsPlsPython::Send2Trash::AggregatedResult>::finished, [this] { accept(); });
}

void FutureWatchDialog::SetFuture(const QFuture<TabsPlsPython::Send2Trash::AggregatedResult>& future) {
    m_watcher.setFuture(future);
}

TabsPlsPython::Send2Trash::AggregatedResult FutureWatchDialog::Result() const { return m_watcher.future().result(); }
