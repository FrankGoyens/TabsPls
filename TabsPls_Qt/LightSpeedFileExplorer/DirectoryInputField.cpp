#include "DirectoryInputField.hpp"

#include <stdexcept>

#include <QLineEdit>

#include "FileSystemDefsConversion.hpp"

#include <TabsPlsCore/FileSystem.hpp>

using FileSystem::StringConversionImpl::ToRawPath;

DirectoryInputField::DirectoryInputField(QString initialDirectory) :
	m_currentDirectory(std::move(initialDirectory))
{
	if (!FileSystem::IsDirectory(ToRawPath(m_currentDirectory)))
		throw std::invalid_argument("The given directory is not valid");

	setText(m_currentDirectory);

	connect(this, &QLineEdit::editingFinished, [this]()
	{
		if (text() == m_currentDirectory)
			return;

		if (!FileSystem::IsDirectory(ToRawPath(text())))
		{
			setText(m_currentDirectory);
			return;
		}

		m_currentDirectory = text();
		emit directoryChanged(m_currentDirectory);
	});
}