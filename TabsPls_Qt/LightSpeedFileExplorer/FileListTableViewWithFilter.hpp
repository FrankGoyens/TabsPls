#pragma once

#include <memory>

#include <QWidget>

class CurrentDirectoryFileOp;
class FileListTableView;
class FileListViewModel;
class QLineEdit;

class FileListTableViewWithFilter : public QWidget {
    Q_OBJECT

  public:
    FileListTableViewWithFilter(std::weak_ptr<CurrentDirectoryFileOp>, FileListViewModel& viewModel);

    FileListTableView& GetFileListTableView() { return *m_fileListTableView; }

    void ClearFilter();

  private:
    FileListTableView* m_fileListTableView;
    QLineEdit* m_filterField;
};