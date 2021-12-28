#include "FileListTableViewWithFilter.hpp"

#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>

#include "FileBrowserViewModelSwitcher.hpp"
#include "FileListViewModel.hpp"
#include "FilterHookedFileListTableView.hpp"
#include "FilterHookedLineEdit.hpp"

static bool PassesFilter(const QString& filter, const QString& name) {
    return name.contains(filter, Qt::CaseInsensitive);
}

static void ShowRowsThatMatchFilter(FileListTableView& tableView, const QAbstractItemModel& model,
                                    const QString& filter) {
    for (int i = 0; i < model.rowCount(); ++i) {
        const auto index = model.index(i, 0);
        const auto name = model.data(index, FileListTableView::GetModelRoleForNames()).toString();

        if (PassesFilter(filter, name))
            tableView.showRow(i);
        else
            tableView.hideRow(i);
    }
}

static void ShowAllRows(FileListTableView& tableView, int rows) {
    for (int i = 0; i < rows; ++i)
        tableView.showRow(i);
}

static auto CreateFilterUpdatedClosure(FileListTableView& tableView) {
    return [&](const QString& filter) {
        if (tableView.model() == nullptr)
            return;

        auto& model = *tableView.model();

        if (filter == "") {
            ShowAllRows(tableView, model.rowCount());
            return;
        }

        ShowRowsThatMatchFilter(tableView, model, filter);
    };
}

static void HideRowRangeUsingFilter(FileListTableView& tableView, const QModelIndex& parentIndex, int first, int last,
                                    const QString& filter) {
    if (tableView.model() == nullptr || filter.isEmpty())
        return;

    for (int row = first; row <= last; ++row) {
        const auto currentIndex = tableView.model()->index(row, 0, parentIndex);
        const auto name = tableView.model()->data(currentIndex, FileListTableView::GetModelRoleForNames()).toString();
        if (!PassesFilter(filter, name))
            tableView.hideRow(row);
    }
}

FileListTableViewWithFilter::FileListTableViewWithFilter(std::shared_ptr<CurrentDirectoryFileOp> currentDirFileOp,
                                                         std::unique_ptr<QAbstractTableModel> viewModel) {
    auto* filterField = new FilterHookedLineEdit;
    m_filterField = filterField;

    m_filterField->hide(); // Hidden when empty

    auto* fileListTableView = new FilterHookedFileListTableView(currentDirFileOp);
    m_fileListTableView = fileListTableView;
    m_fileListTableView->setModel(viewModel.release());
    m_viewModelSwitcher = std::make_shared<FileBrowserViewModelSwitcher>(*m_fileListTableView, currentDirFileOp);

    connect(fileListTableView, &FilterHookedFileListTableView::focusChangeCharacterReceived, [this](char c) {
        if (m_filterField->isHidden())
            m_filterField->show();

        m_filterField->setText(QString(c));
        m_filterField->setFocus();
    });

    auto* vbox = new QVBoxLayout;

    vbox->addWidget(m_fileListTableView);
    vbox->addWidget(m_filterField);

    vbox->setMargin(0);

    setLayout(vbox);

    const auto applyNewFilterClosure = CreateFilterUpdatedClosure(*m_fileListTableView);

    connect(m_filterField, &QLineEdit::textChanged, applyNewFilterClosure);
    connect(m_filterField, &QLineEdit::textChanged, [this](const auto& filter) {
        if (filter == "") {
            m_filterField->hide();
            m_fileListTableView->setFocus();
        }
    });
    connect(m_viewModelSwitcher.get(), &FileBrowserViewModelProvider::rowsInserted,
            [=](const auto& qModelIndex, int first, int last) {
                HideRowRangeUsingFilter(*m_fileListTableView, qModelIndex, first, last, m_filterField->text());
            });

    const auto shiftFocusToTableView = [this]() { m_fileListTableView->setFocus(); };
    connect(filterField, &QLineEdit::returnPressed, [=]() {
        shiftFocusToTableView();
        for (int i = 0; i < m_fileListTableView->model()->rowCount(); ++i)
            if (!m_fileListTableView->isRowHidden(i))
                m_fileListTableView->selectRow(i);
    });
    connect(filterField, &FilterHookedLineEdit::escapePressed, shiftFocusToTableView);

    connect(fileListTableView, &FilterHookedFileListTableView::escapePressed, [this]() { ClearFilter(); });
    connect(m_viewModelSwitcher.get(), &FileBrowserViewModelProvider::modelReset, [this]() { ClearFilter(); });
}

FileListTableViewWithFilter::~FileListTableViewWithFilter() = default;

void FileListTableViewWithFilter::RequestFlatModel() { m_viewModelSwitcher->RequestFlatModel(); }

void FileListTableViewWithFilter::RequestHierarchyModel() { m_viewModelSwitcher->RequestHierarchyModel(); }

std::weak_ptr<FileBrowserViewModelProvider> FileListTableViewWithFilter::GetModelProvider() const {
    return m_viewModelSwitcher;
}

void FileListTableViewWithFilter::ClearFilter() { m_filterField->clear(); }
