#include "FileBrowserViewModelSwitcher.hpp"

#include <QTableView>
#include <QWidget>

#include <TabsPlsCore/CurrentDirectoryFileOp.hpp>
#include <TabsPlsCore/exception/TabsPlsException.hpp>

#include "FileListViewModel.hpp"
#include "FileSystemDefsConversion.hpp"
#include "FlattenedDirectoryViewModel.hpp"

using FileSystem::StringConversion::FromRawPath;

FileBrowserViewModelSwitcher::FileBrowserViewModelSwitcher(
    QTableView& tableView, std::weak_ptr<CurrentDirectoryFileOp> currentDirectoryProvider)
    : m_tableView(tableView), m_currentDirectoryProvider(std::move(currentDirectoryProvider)) {
    QObject::connect(&tableView, &QObject::destroyed, [this] { m_tableDeleted = true; });
}

void FileBrowserViewModelSwitcher::RequestFlatModel() { RequestModel<FlattenedDirectoryViewModel>(); }

void FileBrowserViewModelSwitcher::RequestHierarchyModel() { RequestModel<FileListViewModel>(); }

bool FileBrowserViewModelSwitcher::SwitchingIsPossible() const {
    return !m_tableDeleted && !m_currentDirectoryProvider.expired() && m_tableView.model() != nullptr &&
           dynamic_cast<QWidget*>(m_tableView.model()->parent()) != nullptr;
}

template <typename Model> void FileBrowserViewModelSwitcher::RequestModel() {
    if (SwitchingIsPossible()) {
        auto* currentModelParent = dynamic_cast<QWidget*>(m_tableView.model()->parent());
        auto* model = new Model(currentModelParent, *currentModelParent->style(),
                                FromRawPath(m_currentDirectoryProvider.lock()->GetCurrentDir().path()));
        m_tableView.setModel(model);
    }
}
