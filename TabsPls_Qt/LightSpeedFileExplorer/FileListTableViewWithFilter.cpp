#include "FileListTableViewWithFilter.hpp"

#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>

#include "FileListViewModel.hpp"
#include "FilterHookedFileListTableView.hpp"
#include "FilterHookedLineEdit.hpp"

static void ShowRowsThatMatchFilter(FileListTableView& tableView,
                                    const QAbstractItemModel& model,
                                    const QString& filter) {
    for (int i = 0; i < model.rowCount(); ++i) {
        const auto index = model.index(i, 0);
        const auto name =
            model.data(index, FileListTableView::GetModelRoleForNames())
                .toString();

        if (name.contains(filter, Qt::CaseInsensitive))
            tableView.showRow(i);
        else
            tableView.hideRow(i);
    }
}

static void HideAllRows(FileListTableView& tableView, int rows) {
    for (int i = 0; i < rows; ++i)
        tableView.showRow(i);
}

static auto CreateFilterUpdatedClosure(FileListTableView& tableView) {
    return [&](const QString& filter) {
        if (tableView.model() == nullptr)
            return;

        auto& model = *tableView.model();

        if (filter == "") {
            HideAllRows(tableView, model.rowCount());
            return;
        }

        ShowRowsThatMatchFilter(tableView, model, filter);
    };
}

FileListTableViewWithFilter::FileListTableViewWithFilter(
    std::weak_ptr<CurrentDirectoryFileOp> currentDirFileOp,
    FileListViewModel& viewModel) {
    auto* filterField = new FilterHookedLineEdit;
    m_filterField = filterField;

    m_filterField->hide(); // Hidden when empty

    auto* fileListTableView =
        new FilterHookedFileListTableView(std::move(currentDirFileOp));
    m_fileListTableView = fileListTableView;
    m_fileListTableView->setModel(&viewModel);

    connect(fileListTableView,
            &FilterHookedFileListTableView::focusChangeCharacterReceived,
            [this](char c) {
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

    connect(m_filterField, &QLineEdit::textChanged,
            CreateFilterUpdatedClosure(*m_fileListTableView));
    connect(m_filterField, &QLineEdit::textChanged, [this](const auto& filter) {
        if (filter == "") {
            m_filterField->hide();
            m_fileListTableView->setFocus();
        }
    });

    const auto shiftFocusToTableView = [this]() {
        m_fileListTableView->setFocus();
    };
    connect(filterField, &QLineEdit::returnPressed, [=]() {
        shiftFocusToTableView();
        for (int i = 0; i < m_fileListTableView->model()->rowCount(); ++i)
            if (!m_fileListTableView->isRowHidden(i))
                m_fileListTableView->selectRow(i);
    });
    connect(filterField, &FilterHookedLineEdit::escapePressed,
            shiftFocusToTableView);

    connect(fileListTableView, &FilterHookedFileListTableView::escapePressed,
            [this]() { ClearFilter(); });
    connect(fileListTableView->model(), &QAbstractItemModel::modelReset,
            [this]() { ClearFilter(); });
}

void FileListTableViewWithFilter::ClearFilter() { m_filterField->clear(); }
