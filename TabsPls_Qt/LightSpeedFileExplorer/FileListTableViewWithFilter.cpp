#include "FileListTableViewWithFilter.hpp"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QAbstractItemModel>

#include "FileListTableView.hpp"

static auto CreateFilterUpdatedClosure(FileListTableView& tableView)
{
	return [&](const QString& filter)
	{
		if (tableView.model() == nullptr)
			return;

		auto& model = *tableView.model();

		if (filter == "") {
			for (int i = 0; i < model.rowCount(); ++i)
				tableView.showRow(i);
			return;
		}

		for (int i = 0; i < model.rowCount(); ++i)
		{
			const auto index = model.index(i, 0);
			const auto name = model.data(index, FileListTableView::GetModelRoleForNames()).toString();

			if (name.contains(filter, Qt::CaseInsensitive))
				tableView.showRow(i);
			else
				tableView.hideRow(i);
		}
	};
}

FileListTableViewWithFilter::FileListTableViewWithFilter()
{
	m_fileListTableView = new FileListTableView;
	m_filterField = new QLineEdit;

	auto* vbox = new QVBoxLayout;

	vbox->addWidget(m_fileListTableView);
	vbox->addWidget(m_filterField);

	setLayout(vbox);

	connect(m_filterField, &QLineEdit::textChanged, CreateFilterUpdatedClosure(*m_fileListTableView));
}