#pragma once

#include <memory>

class QTableView;
class QWidget;
class CurrentDirectoryFileOp;

class FileBrowserViewModelSwitcher {

  public:
    /**! \brief Assumes the table view already has a model, that model's parent will be used a parent when requesting
     * other models
     */
    FileBrowserViewModelSwitcher(QTableView&, std::weak_ptr<CurrentDirectoryFileOp>);

    void RequestFlatModel();
    void RequestHierarchyModel();

  private:
    bool SwitchingIsPossible() const;

    template <typename Model> void RequestModel();

    bool m_tableDeleted = false;
    QTableView& m_tableView;
    std::weak_ptr<CurrentDirectoryFileOp> m_currentDirectoryProvider;
};
