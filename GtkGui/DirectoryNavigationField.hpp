#pragma once

typedef struct _GtkWidget GtkWidget;

namespace FileSystem
{
	class Directory;
}

namespace DirectoryNavigationField
{
	GtkWidget* BuildDirectoryNavigationField(const FileSystem::Directory& dir);
}