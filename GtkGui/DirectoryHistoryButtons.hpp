#pragma once

#include <memory>

typedef struct _GtkWidget GtkWidget;

namespace DirectoryHistoryButtons
{
	struct InternalUserdata
	{
		virtual ~InternalUserdata() = default;
	};

	struct DirectoryHistoryButtonWidget
	{
		GtkWidget& widget;

		std::unique_ptr<InternalUserdata> _internalUserdata;
	};

	DirectoryHistoryButtonWidget BuildDirectoryHistoryButtons();
}