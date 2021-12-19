#pragma once

#include <memory>

#include <QWidget>

class CurrentDirectoryFileOp;
class FileListTableView;
class QAbstractTableModel;
class QLineEdit;
class FileBrowserViewModelSwitcher;

class FileListTableViewWithFilter final : public QWidget {
    Q_OBJECT

  public:
    FileListTableViewWithFilter(std::shared_ptr<CurrentDirectoryFileOp>, QAbstractTableModel& viewModel);
    ~FileListTableViewWithFilter();

    void RequestFlatModel();
    void RequestHierarchyModel();

    FileListTableView& GetFileListTableView() { return *m_fileListTableView; }

    void ClearFilter();

  private:
    FileListTableView* m_fileListTableView;
    QLineEdit* m_filterField;
    std::unique_ptr<FileBrowserViewModelSwitcher> m_viewModelSwitcher;
};