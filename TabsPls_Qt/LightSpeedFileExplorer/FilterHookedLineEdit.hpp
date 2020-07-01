#pragma once

#include <QLineEdit>

class FilterHookedLineEdit : public QLineEdit
{
	Q_OBJECT

signals:
	void escapePressed();
	
protected:
	void keyPressEvent(QKeyEvent*) override;
};