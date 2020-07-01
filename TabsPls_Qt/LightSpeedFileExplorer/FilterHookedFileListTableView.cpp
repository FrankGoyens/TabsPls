#include "FilterHookedFileListTableView.hpp"

#include <locale>

#include <QKeyEvent>

static bool CharacterWouldShiftFocus(char c)
{
	return std::isalnum(c) || c == '.';
}

void FilterHookedFileListTableView::keyPressEvent(QKeyEvent* event)
{
	const auto c = event->text().toStdString()[0];
	if (CharacterWouldShiftFocus(c))
		emit focusChangeCharacterReceived(c);
	else
	{
		if (event->key() == Qt::Key_Escape)
			emit escapePressed();
		FileListTableView::keyPressEvent(event);
	}
}
