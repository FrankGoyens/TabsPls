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

    if (auto* model = GetActiveModel()) {
        QObject::connect(model, &QAbstractTableModel::modelReset, [this] { emit modelReset(); });
        QObject::connect(
            model, &QAbstractTableModel::rowsInserted,
            [this](const QModelIndex& parent, int first, int last) { emit rowsInserted(parent, first, last); });
    }
}

void FileBrowserViewModelSwitcher::RequestFlatModel() { RequestModel<FlattenedDirectoryViewModel>(); }

void FileBrowserViewModelSwitcher::RequestHierarchyModel() { RequestModel<FileListViewModel>(); }

QAbstractTableModel* FileBrowserViewModelSwitcher::GetActiveModel() const {
    if (m_tableDeleted || m_tableView.model() == nullptr)
        return nullptr;
    return dynamic_cast<QAbstractTableModel*>(m_tableView.model());
}

DirectoryChanger* FileBrowserViewModelSwitcher::GetDirectoryChangerForActiveModel() const {
    return dynamic_cast<DirectoryChanger*>(GetActiveModel());
}

bool FileBrowserViewModelSwitcher::SwitchingIsPossible() const {
    return !m_tableDeleted && !m_currentDirectoryProvider.expired() && m_tableView.model() != nullptr &&
           dynamic_cast<QWidget*>(m_tableView.model()->parent()) != nullptr;
}

template <typename Model> void FileBrowserViewModelSwitcher::RequestModel() {
    if (SwitchingIsPossible()) {
        auto* currentModel = m_tableView.model();
        auto* currentModelParent = dynamic_cast<QWidget*>(currentModel->parent());
        auto* model = new Model(currentModelParent, *currentModelParent->style(),
                                FromRawPath(m_currentDirectoryProvider.lock()->GetCurrentDir().path()));
        m_tableView.setModel(model);
        QObject::connect(model, &QAbstractTableModel::modelReset, [this] { emit modelReset(); });
        QObject::connect(
            model, &QAbstractTableModel::rowsInserted,
            [this](const QModelIndex& parent, int first, int last) { emit rowsInserted(parent, first, last); });
        delete currentModel;
    }
}
