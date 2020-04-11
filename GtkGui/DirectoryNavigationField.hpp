#pragma once

#include <memory>
#include <GtkGui/FileListView.hpp>
#include <CurrentDirectoryProvider.hpp>

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

	struct DirectoryNavigationFieldWidget : CurrentDirectoryProvider
	{
		DirectoryNavigationFieldWidget(GtkWidget& widget_, std::unique_ptr<InternalUserdata> internalUserdata_) :
			widget(widget_),
			_internalUserdata(std::move(internalUserdata_))
		{}

		void RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>&);
		const FileSystem::Directory& Get() const override;

		GtkWidget& widget;
		std::unique_ptr<InternalUserdata> _internalUserdata;
	};

	DirectoryNavigationFieldWidget BuildDirectoryNavigationField(const FileSystem::Directory& dir);

	std::unique_ptr<FileListView::DirectoryChangedAction> CreateDirectoryChangedCallback(DirectoryNavigationFieldWidget& widgetWithStore);
}