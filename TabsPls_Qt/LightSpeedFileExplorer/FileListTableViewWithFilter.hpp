#pragma once

#include <memory>

#include <QWidget>

class CurrentDirectoryFileOp;
class FileListTableView;
class QAbstractTableModel;
class QLineEdit;
class FileBrowserViewModelSwitcher;
class FileBrowserViewModelProvider;

class FileListTableViewWithFilter final : public QWidget {
    Q_OBJECT

  public:
    FileListTableViewWithFilter(std::shared_ptr<CurrentDirectoryFileOp>,
                                std::unique_ptr<QAbstractTableModel> viewModel);
    ~FileListTableViewWithFilter();

    void RequestFlatModel();
    void RequestHierarchyModel();

    FileListTableView& GetFileListTableView() { return *m_fileListTableView; }
    std::weak_ptr<FileBrowserViewModelProvider> GetModelProvider() const;

    void ClearFilter();

  private:
    FileListTableView* m_fileListTableView;
    QLineEdit* m_filterField;
    std::shared_ptr<FileBrowserViewModelSwitcher> m_viewModelSwitcher;
};