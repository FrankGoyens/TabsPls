#include "TabsPlsMainWindow.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>

#include "DirectoryInputField.hpp"
#include "FileListTableView.hpp"
#include "FileListViewModel.hpp"

static auto SetupTopBar(QWidget& widget, const QString& initialDirectory)
{
	auto* vbox = new QHBoxLayout();
	auto* backButton = new QPushButton(u8"←");
	auto* forwardButton = new QPushButton(u8"→");
	auto* directoryInputField = new DirectoryInputField(initialDirectory);

	backButton->setFixedWidth(50);
	forwardButton->setFixedWidth(50);

	vbox->addWidget(backButton);
	vbox->addWidget(forwardButton);

	vbox->addWidget(directoryInputField);
	vbox->setMargin(0);

	widget.setLayout(vbox);

	return directoryInputField;
}

static auto SetupCentralWidget(const QString& initialDirectory)
{
	auto* centralWidget = new QWidget();

	auto* topBarWidget = new QWidget();
	auto* topBarDirectoryInputField = SetupTopBar(*topBarWidget, initialDirectory);

	auto* fileListViewWidget = new FileListTableView();

	auto* rootLayout = new QVBoxLayout();
	rootLayout->addWidget(topBarWidget);
	rootLayout->addWidget(fileListViewWidget);

	centralWidget->setLayout(rootLayout);

	return std::make_tuple( centralWidget, fileListViewWidget, topBarDirectoryInputField);
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory)
{
	setWindowTitle(tr("Light Speed File Explorer"));

	const auto[centralWidget, fileListViewWidget, topBarDirectoryInputField] = SetupCentralWidget(initialDirectory);
	setCentralWidget(centralWidget);

	auto* fileListViewModel = new FileListViewModel(initialDirectory);
	fileListViewWidget->setModel(fileListViewModel);

	connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged, [=](const auto& dirPath) {fileListViewModel->ChangeDirectory(dirPath); });
}