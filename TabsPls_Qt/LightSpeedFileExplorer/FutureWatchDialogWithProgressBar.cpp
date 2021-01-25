#include "FutureWatchDialogWithProgressBar.hpp"

#include <QProgressBar>
#include <QVBoxLayout>

#include "QObjectProgressReport.hpp"

QProgressBar& FutureWatchDialogWithProgressBar::MakeSingleProgressBarFutureDialog(QWidget& futureDialog) {
    auto [progressBar, rootLayout] = FutureWatchDialogWithProgressBar::MakeProgressBar();

    futureDialog.setLayout(&rootLayout);
    futureDialog.adjustSize();

    return progressBar;
}

std::tuple<QProgressBar&, QVBoxLayout&> FutureWatchDialogWithProgressBar::MakeProgressBar() {

    auto* progressBar = new QProgressBar();

    auto* rootLayout = new QVBoxLayout();
    rootLayout->addWidget(progressBar);

    return {*progressBar, *rootLayout};
}

void FutureWatchDialogWithProgressBar::ConnectProgressReportToProgressBar(const QObjectProgressReport& reporter,
                                                                          QProgressBar& progressBar) {
    QObject::connect(&reporter, &QObjectProgressReport::Updated, &progressBar, &QProgressBar::setValue,
                     Qt::ConnectionType::QueuedConnection);
    QObject::connect(&reporter, &QObjectProgressReport::MinimumSet, &progressBar, &QProgressBar::setMinimum,
                     Qt::ConnectionType::QueuedConnection);
    QObject::connect(&reporter, &QObjectProgressReport::MaximumSet, &progressBar, &QProgressBar::setMaximum,
                     Qt::ConnectionType::QueuedConnection);
}
