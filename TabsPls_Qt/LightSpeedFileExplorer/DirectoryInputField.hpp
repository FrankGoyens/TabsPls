#pragma once

#include <QLineEdit>
#include <QString>

class QLineEdit;

class DirectoryInputField : public QLineEdit
{
	Q_OBJECT
public:
	DirectoryInputField(QString initialDirectory);

signals:
	void directoryChanged(QString);

private:
	QString m_currentDirectory;
};