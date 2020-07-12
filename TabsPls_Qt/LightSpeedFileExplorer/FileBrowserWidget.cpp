#include "FileBrowserWidget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QShortcut>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

#include "DirectoryInputField.hpp"
#include "FileListTableView.hpp"
#include "FileListTableViewWithFilter.hpp"
#include "FileListViewModel.hpp"
#include "CurrentDirectoryFileOpQtImpl.hpp"

namespace
{
	using CurrentDirSetter = std::function<void(FileSystem::Directory)>;
}

static void ChangeDirectoryWithoutHistoryUpdate(const FileSystem::Directory& newDir, 
	FileListTableViewWithFilter& fileListViewWithFilter, 
	FileListViewModel& model, 
	QLineEdit& directoryInputField,
	CurrentDirSetter currentDirSetter)
{
	const auto newDirString = QString::fromStdString(newDir.path());
	directoryInputField.setText(newDirString);
	model.ChangeDirectory(newDirString);
	fileListViewWithFilter.ClearFilter();
	currentDirSetter(newDir);
}

static auto CreateDirectoryChangedByGoingToParent(RobustDirectoryHistoryStore& history, 
	FileListTableViewWithFilter& fileListViewWithFilter, 
	FileListViewModel& model, 
	QLineEdit& directoryInputField, 
	CurrentDirSetter currentDirSetter)
{
	return [&, currentDirSetter]()
	{
		try
		{
			const auto newDir = history.GetCurrent().Parent();
			if (newDir.path() == history.GetCurrent().path())
				return;
			history.OnNewDirectory(newDir);
			ChangeDirectoryWithoutHistoryUpdate(newDir, fileListViewWithFilter, model, directoryInputField, currentDirSetter);
		}
		catch (const StoreIsEmptyException&) {}
	};
}

static auto CreateDirectoryChangedClosure(RobustDirectoryHistoryStore& history, 
	FileListTableViewWithFilter& fileListViewWithFilter, 
	FileListViewModel& model, 
	QLineEdit& directoryInputField, 
	CurrentDirSetter currentDirSetter)
{
	return [&, currentDirSetter](const FileSystem::Directory& newDir)
	{
		history.OnNewDirectory(newDir);
		ChangeDirectoryWithoutHistoryUpdate(newDir, fileListViewWithFilter, model, directoryInputField, currentDirSetter);
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
static auto CreateHistoryActionClosure(RobustDirectoryHistoryStore& history, 
	FileListTableViewWithFilter& fileListViewWithFilter, 
	FileListViewModel& model, 
	QLineEdit& directoryInputField, 
	CurrentDirSetter currentDirSetter,
	const HistoryVariant& variant)
{
	return [&, currentDirSetter]()
	{
		try
		{
			SwitchHistory(history, variant);
			ChangeDirectoryWithoutHistoryUpdate(history.GetCurrent(), fileListViewWithFilter, model, directoryInputField, currentDirSetter);
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

static auto SetupCentralWidget(QWidget& fileBrowserWidget, std::weak_ptr<CurrentDirectoryFileOp> currentDirFileOp, FileListViewModel& viewModel, const QString& initialDirectory)
{
	auto* topBarWidget = new QWidget();
	auto[backButton, forwardButton, topBarDirectoryInputField] = SetupTopBar(*topBarWidget, initialDirectory);

	auto* fileListViewWidget = new FileListTableViewWithFilter(std::move(currentDirFileOp), viewModel);

	auto* fileListViewActiveFilterLabel = new QLabel();

	auto* rootLayout = new QVBoxLayout();
	rootLayout->addWidget(topBarWidget);
	rootLayout->addWidget(fileListViewWidget);
	rootLayout->addWidget(fileListViewActiveFilterLabel);

	fileBrowserWidget.setLayout(rootLayout);

	return std::make_tuple( fileListViewWidget, topBarDirectoryInputField, backButton, forwardButton);
}

FileBrowserWidget::FileBrowserWidget(FileSystem::Directory initialDir):
	m_currentDirectory(std::move(initialDir))
{
	m_historyStore.OnNewDirectory(m_currentDirectory);

	m_currentDirFileOpImpl = std::make_shared<CurrentDirectoryFileOpQtImpl>(m_currentDirectory);

	auto* fileListViewModel = new FileListViewModel(*style(), m_currentDirectory.path().c_str());

	const auto [fileListViewWidget_from_binding, topBarDirectoryInputField_from_binding, backButton, forwardButton] = SetupCentralWidget(*this, m_currentDirFileOpImpl, *fileListViewModel, m_currentDirectory.path().c_str());

	auto* fileListViewWidget = fileListViewWidget_from_binding; //This is done to be able to capture fileListViewWidget in lambda's
	auto* topBarDirectoryInputField = topBarDirectoryInputField_from_binding;

	const auto setCurrentDirectoryMemberCall = [this](FileSystem::Directory newDir) {SetCurrentDirectory(std::move(newDir)); };

	const auto directoryChangedClosure = CreateDirectoryChangedClosure(m_historyStore, 
		*fileListViewWidget, 
		*fileListViewModel, 
		*topBarDirectoryInputField,
		setCurrentDirectoryMemberCall);

	connect(topBarDirectoryInputField, &DirectoryInputField::directoryChanged, [=](const auto& dirString)
	{
		if (const auto dir = FileSystem::Directory::FromPath(dirString.toStdString()))
			directoryChangedClosure(*dir);
	});

	const auto backActionClosure = CreateHistoryActionClosure(m_historyStore, 
		*fileListViewWidget, 
		*fileListViewModel, 
		*topBarDirectoryInputField, 
		setCurrentDirectoryMemberCall,
		HistoryBackVariant());

	const auto forwardActionClosure = CreateHistoryActionClosure(m_historyStore, 
		*fileListViewWidget, 
		*fileListViewModel, 
		*topBarDirectoryInputField, 
		setCurrentDirectoryMemberCall,
		HistoryForwardVariant());

	backButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Left));
	forwardButton->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Right));

	connect(backButton, &QPushButton::pressed, backActionClosure);
	connect(forwardButton, &QPushButton::pressed, forwardActionClosure);

	const auto directoryChangedByGoingToParentClosure = CreateDirectoryChangedByGoingToParent(m_historyStore, 
		*fileListViewWidget, 
		*fileListViewModel, 
		*topBarDirectoryInputField,
		setCurrentDirectoryMemberCall);

	connect(&fileListViewWidget->GetFileListTableView(), &QTableView::activated, [=](const QModelIndex& index)
	{
		const auto dirString = fileListViewModel->data(index, Qt::UserRole);

		if (dirString == "..")
			directoryChangedByGoingToParentClosure();
		else if (const auto dir = FileSystem::Directory::FromPath(dirString.toString().toStdString()))
			directoryChangedClosure(*dir);
		else if (FileSystem::IsRegularFile(dirString.toString().toStdString())) 
			QDesktopServices::openUrl(QUrl::fromLocalFile(dirString.toString()));
	});

	const auto* gotoParentActionShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Up), this);
	connect(gotoParentActionShortcut, &QShortcut::activated, directoryChangedByGoingToParentClosure);

	const auto* focusAndSelectDirectoryInputField = new QShortcut(QKeySequence(Qt::Key_F6), this);
	connect(focusAndSelectDirectoryInputField, &QShortcut::activated, [=]()
	{
		topBarDirectoryInputField->setFocus();
		topBarDirectoryInputField->selectAll();
	});
}

const QString FileBrowserWidget::GetCurrentDirectoryName() const
{
	return FileSystem::GetDirectoryname(m_currentDirectory).c_str();
}

void FileBrowserWidget::SetCurrentDirectory(FileSystem::Directory newDir)
{
	m_currentDirectory = std::move(newDir);
	m_currentDirFileOpImpl->updateCurrentDir(m_currentDirectory);
	emit currentDirectoryNameChanged(GetCurrentDirectoryName());
}
