#pragma once

#include <memory>

#include "FileListTableView.hpp"

class CurrentDirectoryFileOp;

class FilterHookedFileListTableView : public FileListTableView {
    Q_OBJECT
  public:
    FilterHookedFileListTableView(std::weak_ptr<CurrentDirectoryFileOp>);

  signals:
    void focusChangeCharacterReceived(char);
    void escapePressed();

  protected:
    void keyPressEvent(QKeyEvent* event) override;
};
