#include "DirectoryInputField.hpp"

#include <QLineEdit>

#include <TabsPlsCore/FileSystem.hpp>

DirectoryInputField::DirectoryInputField(QString initialDirectory) :
	m_currentDirectory(std::move(initialDirectory))
{
	if (!FileSystem::IsDirectory(m_currentDirectory.toStdString()))
		throw std::invalid_argument("The given directory is not valid");

	setText(m_currentDirectory);

	connect(this, &QLineEdit::editingFinished, [this]()
	{
		if (!FileSystem::IsDirectory(text().toStdString()))
		{
			setText(m_currentDirectory);
			return;
		}

		m_currentDirectory = text();
		emit directoryChanged(m_currentDirectory);
	});
}