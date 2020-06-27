#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QShortcut>

#include "DirectoryInputField.hpp"
#include "FileListTableView.hpp"
#include "FileListViewModel.hpp"

namespace
{
	struct HistoryBackVariant {};
	struct HistoryForwardVariant {};
}

static void SwitchHistory(RobustDirectoryHistoryStore& history, const HistoryBackVariant&)
{
	history.SwitchToPrevious();
}

static void SwitchHistory(RobustDirectoryHistoryStore& history, const HistoryForwardVariant&)
{
	history.SwitchToNext();
}

template<typename HistoryVariant>
static auto CreateHistoryActionClosure(RobustDirectoryHistoryStore& history, FileListViewModel& model, QLineEdit& directoryInputField, const HistoryVariant& variant)
{
	return [&]()
	{
		try
		{
			SwitchHistory(history, variant);
			const auto newDir = QString::fromStdString(history.GetCurrent().path());
			directoryInputField.setText(newDir);
			model.ChangeDirectory(newDir);
		}
		catch (const ImpossibleSwitchException&) {}
	};
}

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

	return std::make_tuple(backButton, forwardButton, directoryInputField);
}

static auto SetupCentralWidget(const QString& initialDirectory)
{
	auto* centralWidget = new QWidget();

	auto* topBarWidget = new QWidget();
	auto[backButton, forwardButton, topBarDirectoryInputField] = SetupTopBar(*topBarWidget, initialDirectory);

	auto* fileListViewWidget = new FileListTableView();

	auto* rootLayout = new QVBoxLayout();
	rootLayout->addWidget(topBarWidget);
	rootLayout->addWidget(fileListViewWidget);

	centralWidget->setLayout(rootLayout);

	return std::make_tuple( centralWidget, fileListViewWidget, topBarDirectoryInputField, backButton, forwardButton);
}

TabsPlsMainWindow::TabsPlsMainWindow(const QString& initialDirectory)
{
	setWindowTitle(tr("Light Speed File Explorer"));

	const auto validInitialDir = FileSystem::Directory::FromPath(initialDirectory.toStdString());

	if (!validInitialDir)
		throw std::invalid_argument("The given directory does not exist");

	m_historyStore.OnNewDirectory(*validInitialDir);

	const auto[centralWidget, fileListViewWidget, topBarDirectoryInputField, backButton, forwardButton] = SetupCentralWidget(initialDirectory);
	setCentralWidget(centralWidget);

	auto* fileListViewModel = new FileListViewModel(initialDirectory);
	fileListViewWidget->setModel(fileListViewModel);

	connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged, [=](const auto& dirPath) {fileListViewModel->ChangeDirectory(dirPath); });
	connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged, [=](const auto& dirPath) 
	{
		if (const auto dir = FileSystem::Directory::FromPath(dirPath.toStdString()))
			m_historyStore.OnNewDirectory(*dir);
	});

	const auto backActionClosure = CreateHistoryActionClosure(m_historyStore, *fileListViewModel, *topBarDirectoryInputField, HistoryBackVariant());
	const auto forwardActionClosure = CreateHistoryActionClosure(m_historyStore, *fileListViewModel, *topBarDirectoryInputField, HistoryForwardVariant());

	backButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	forwardButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));

	connect(backButton, &QPushButton::pressed, backActionClosure);
	connect(forwardButton, &QPushButton::pressed, forwardActionClosure);
}