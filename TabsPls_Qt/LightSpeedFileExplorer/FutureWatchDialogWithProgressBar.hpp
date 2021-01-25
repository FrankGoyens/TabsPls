#pragma once

#include <utility>

class QObject;
class QProgressBar;
class QVBoxLayout;
class QObjectProgressReport;
class QWidget;

namespace FutureWatchDialogWithProgressBar {
QProgressBar& MakeSingleProgressBarFutureDialog(QWidget& futureDialog);
std::tuple<QProgressBar&, QVBoxLayout&> MakeProgressBar();
void ConnectProgressReportToProgressBar(const QObjectProgressReport& reporter, QProgressBar& progressBar);
} // namespace FutureWatchDialogWithProgressBar
