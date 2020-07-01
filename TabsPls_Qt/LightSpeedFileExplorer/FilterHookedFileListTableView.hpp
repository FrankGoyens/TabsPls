#pragma once

#include "FileListTableView.hpp"

class FilterHookedFileListTableView : public FileListTableView
{
	Q_OBJECT

signals:
	void focusChangeCharacterReceived(char);
	void escapePressed();

protected:
	void keyPressEvent(QKeyEvent* event) override;
};
