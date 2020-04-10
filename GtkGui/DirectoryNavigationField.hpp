#pragma once

#include <memory>
#include <GtkGui/FileListView.hpp>

typedef struct _GtkWidget GtkWidget;

namespace FileSystem
{
	class Directory;
}

namespace FileListView
{
	struct DirectoryChangedAction;
}

namespace DirectoryNavigationField
{
	/*! \brief This is used to exchange data internally, this should be kept in scope as long as the list widget is in scope*/
	struct InternalUserdata
	{
		virtual ~InternalUserdata() = default;
	};

	struct DirectoryChangedAction
	{
		virtual ~DirectoryChangedAction() = default;
		virtual void Do(const FileSystem::Directory& dir) = 0;
	};

	struct DirectoryNavigationFieldWidget
	{
		GtkWidget& widget;

		void RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>&);

		std::unique_ptr<InternalUserdata> _internalUserdata;
	};

	DirectoryNavigationFieldWidget BuildDirectoryNavigationField(const FileSystem::Directory& dir);

	std::unique_ptr<FileListView::DirectoryChangedAction> CreateDirectoryChangedCallback(DirectoryNavigationFieldWidget& widgetWithStore);
}