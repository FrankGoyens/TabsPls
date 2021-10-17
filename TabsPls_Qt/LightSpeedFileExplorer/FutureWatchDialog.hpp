#pragma once

#include <memory>
#include <variant>

#include <QDialog>
#include <QFutureWatcher>

namespace TabsPlsPython::Send2Trash {
struct AggregatedResult;
}
class QObjectProgressReport;
class QProgressBar;
class QStringList;

class FutureWatchDialog : public QDialog {
    Q_OBJECT
  public:
    using ResultValue = std::variant<std::monostate, std::shared_ptr<TabsPlsPython::Send2Trash::AggregatedResult>,
                                     std::shared_ptr<QStringList>>;

    FutureWatchDialog(QWidget* parent, const QString& title);

    void SetFuture(const QFuture<ResultValue>& future) { m_watcher.setFuture(future); }
    void ConnectProgressReporterFromAnotherThread(std::shared_ptr<QObjectProgressReport>);

    ResultValue Result() { return m_watcher.future().result(); }

    template <typename Func> void ConnectFunctionToFutureFinish(const Func& function) {
        QObject::connect(&m_watcher, &QFutureWatcher<ResultValue>::finished, function);
    }

    const QFutureWatcher<ResultValue>& GetWatcher() const { return m_watcher; }

  private:
    QFutureWatcher<ResultValue> m_watcher;
    std::shared_ptr<QObjectProgressReport> m_progressReport = nullptr;
    QProgressBar* m_progressBar = nullptr;
};