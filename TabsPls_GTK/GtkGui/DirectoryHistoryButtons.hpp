#pragma once

#include <memory>

#include <DirectoryHistory.hpp>

#include <Gui/DirectoryChangedAction.hpp>

typedef struct _GtkWidget GtkWidget;

namespace FileListView
{
	struct DirectoryChangedAction;
}

namespace DirectoryNavigationField
{
	struct DirectoryChangedAction;
}

struct CurrentDirectoryProvider;

namespace DirectoryHistoryButtons
{
	struct InternalUserdata
	{
		virtual ~InternalUserdata() = default;
	};
	
	struct DirectoryChangedAction : Gui::DirectoryChangedAction {};

	struct DirectoryHistoryButtonWidget : DirectoryHistory
	{
		DirectoryHistoryButtonWidget(GtkWidget& widget_, std::unique_ptr<InternalUserdata> internalUserdata_):
			widget(widget_),
			_internalUserdata(std::move(internalUserdata_))
		{}

		void RegisterDirectoryChanged(const std::weak_ptr<DirectoryChangedAction>&);

		void RequestPreviousDirectory() const override;
		void RequestNextDirectory() const override;

		GtkWidget& widget;
		std::unique_ptr<InternalUserdata> _internalUserdata;
	};

	DirectoryHistoryButtonWidget BuildDirectoryHistoryButtons(const CurrentDirectoryProvider&);

	std::unique_ptr<FileListView::DirectoryChangedAction> CreateDirectoryChangedCallback_FileListView(DirectoryHistoryButtonWidget&);
	std::unique_ptr<DirectoryNavigationField::DirectoryChangedAction> CreateDirectoryChangedCallback_DirectoryNavigationField(DirectoryHistoryButtonWidget&);
}