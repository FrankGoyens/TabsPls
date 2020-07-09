#include "TabsPlsMainWindow.hpp"

#include <stdexcept>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QShortcut>
#include <QLabel>

#include "DirectoryInputField.hpp"
#include "FileListTableView.hpp"
#include "FileListTableViewWithFilter.hpp"
#include "FileListViewModel.hpp"
#include "CurrentDirectoryFileOpQtImpl.hpp"

static void ChangeDirectoryWithoutHistoryUpdate(const FileSystem::Directory& newDir, FileListTableViewWithFilter& fileListViewWithFilter, FileListViewModel& model, QLineEdit& directoryInputField, CurrentDirectoryFileOpQtImpl& currentDirFileOpImpl)
{
	const auto newDirString = QString::fromStdString(newDir.path());
	directoryInputField.setText(newDirString);
	model.ChangeDirectory(newDirString);
	fileListViewWithFilter.ClearFilter();
	currentDirFileOpImpl.updateCurrentDir(newDir);
}

static auto CreateDirectoryChangedByGoingToParent(RobustDirectoryHistoryStore& history, FileListTableViewWithFilter& fileListViewWithFilter, FileListViewModel& model, QLineEdit& directoryInputField, CurrentDirectoryFileOpQtImpl& currentDirFileOpImpl)
{
	return [&]()
	{
		try
		{
			const auto newDir = history.GetCurrent().Parent();
			if (newDir.path() == history.GetCurrent().path())
				return;
			history.OnNewDirectory(newDir);
			ChangeDirectoryWithoutHistoryUpdate(newDir, fileListViewWithFilter, model, directoryInputField, currentDirFileOpImpl);
		}
		catch (const StoreIsEmptyException&) {}
	};
}

static auto CreateDirectoryChangedClosure(RobustDirectoryHistoryStore& history, FileListTableViewWithFilter& fileListViewWithFilter, FileListViewModel& model, QLineEdit& directoryInputField, CurrentDirectoryFileOpQtImpl& currentDirFileOpImpl)
{
	return [&](const FileSystem::Directory& newDir)
	{
		history.OnNewDirectory(newDir);
		ChangeDirectoryWithoutHistoryUpdate(newDir, fileListViewWithFilter, model, directoryInputField, currentDirFileOpImpl);
	};
}

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
static auto CreateHistoryActionClosure(RobustDirectoryHistoryStore& history, FileListTableViewWithFilter& fileListViewWithFilter, FileListViewModel& model, QLineEdit& directoryInputField, CurrentDirectoryFileOpQtImpl& currentDirFileOpImpl, const HistoryVariant& variant)
{
	return [&]()
	{
		try
		{
			SwitchHistory(history, variant);
			ChangeDirectoryWithoutHistoryUpdate(history.GetCurrent(), fileListViewWithFilter, model, directoryInputField, currentDirFileOpImpl);
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

static auto SetupCentralWidget(std::weak_ptr<CurrentDirectoryFileOp> currentDirFileOp, const QString& initialDirectory)
{
	auto* centralWidget = new QWidget();

	auto* topBarWidget = new QWidget();
	auto[backButton, forwardButton, topBarDirectoryInputField] = SetupTopBar(*topBarWidget, initialDirectory);

	auto* fileListViewWidget = new FileListTableViewWithFilter(std::move(currentDirFileOp));

	auto* fileListViewActiveFilterLabel = new QLabel();

	auto* rootLayout = new QVBoxLayout();
	rootLayout->addWidget(topBarWidget);
	rootLayout->addWidget(fileListViewWidget);
	rootLayout->addWidget(fileListViewActiveFilterLabel);

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

	auto currentDirFileOpImpl = std::make_shared<CurrentDirectoryFileOpQtImpl>(*validInitialDir);
	m_currentDirFileOpImpl = currentDirFileOpImpl;

	const auto[centralWidget, fileListViewWidget_from_binding, topBarDirectoryInputField, backButton, forwardButton] = SetupCentralWidget(currentDirFileOpImpl, initialDirectory);
	setCentralWidget(centralWidget);

	auto* fileListViewModel = new FileListViewModel(initialDirectory);
	auto* fileListViewWidget = fileListViewWidget_from_binding; //This is done to be able to capture this in lambda's
	fileListViewWidget->GetFileListTableView().setModel(fileListViewModel);

	const auto directoryChangedClosure = CreateDirectoryChangedClosure(m_historyStore, *fileListViewWidget, *fileListViewModel, *topBarDirectoryInputField, *currentDirFileOpImpl);

	connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged, [=](const auto& dirString) 
	{
		if (const auto dir = FileSystem::Directory::FromPath(dirString.toStdString()))
			directoryChangedClosure(*dir);
	});

	const auto backActionClosure = CreateHistoryActionClosure(m_historyStore, *fileListViewWidget, *fileListViewModel, *topBarDirectoryInputField, *currentDirFileOpImpl, HistoryBackVariant());
	const auto forwardActionClosure = CreateHistoryActionClosure(m_historyStore, *fileListViewWidget, *fileListViewModel, *topBarDirectoryInputField, *currentDirFileOpImpl, HistoryForwardVariant());

	backButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	forwardButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));

	connect(backButton, &QPushButton::pressed, backActionClosure);
	connect(forwardButton, &QPushButton::pressed, forwardActionClosure);

	const auto directoryChangedByGoingToParentClosure = CreateDirectoryChangedByGoingToParent(m_historyStore, *fileListViewWidget, *fileListViewModel, *topBarDirectoryInputField, *currentDirFileOpImpl);

	connect(&fileListViewWidget->GetFileListTableView(), &QTableView::activated, [=](const QModelIndex& index)
	{
		const auto dirString = fileListViewModel->data(index, Qt::UserRole);

		if (dirString == "..")
			directoryChangedByGoingToParentClosure();
		else if (const auto dir = FileSystem::Directory::FromPath(dirString.toString().toStdString()))
			directoryChangedClosure(*dir);
	});

	const auto* gotoParentActionShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Up), this);

	connect(gotoParentActionShortcut, &QShortcut::activated, directoryChangedByGoingToParentClosure);
}