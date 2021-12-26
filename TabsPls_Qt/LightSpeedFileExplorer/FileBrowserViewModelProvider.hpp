#pragma once

#include <QObject>

class QAbstractTableModel;
struct DirectoryChanger;

class FileBrowserViewModelProvider : public QObject {
    Q_OBJECT
  public:
    virtual ~FileBrowserViewModelProvider() = default;
    virtual QAbstractTableModel* GetActiveModel() const = 0;
    virtual DirectoryChanger* GetDirectoryChangerForActiveModel() const = 0;

  signals:
    void modelReset();
    void rowsInserted(const QModelIndex&, int, int);
};
