#pragma once

#include <memory>

#include <QFutureWatcher>

template <typename ResultT> class FutureWatchDialog {
  public:
    void SetFuture(const QFuture<ResultT>&);
    ResultT Result();

    template <typename Func> void ConnectFunctionToFutureFinish(Func);

  private:
    QFutureWatcher<ResultT> m_watcher;
};

template <typename ResultT> inline void FutureWatchDialog<ResultT>::SetFuture(const QFuture<ResultT>& future) {
    m_watcher.setFuture(future);
}

template <typename ResultT> inline ResultT FutureWatchDialog<ResultT>::Result() { return m_watcher.future().result(); }

template <typename ResultT>
template <typename Func>
inline void FutureWatchDialog<ResultT>::ConnectFunctionToFutureFinish(Func function) {
    QObject::connect(&m_watcher, &QFutureWatcher<ResultT>::finished, function);
}
