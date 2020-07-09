#pragma once

#include <memory>

#include <QTableView>

class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;
class QLabel;

class CurrentDirectoryFileOp;

class FileListTableView: public QTableView
{
	Q_OBJECT

public:
	FileListTableView(std::weak_ptr<CurrentDirectoryFileOp>);

	static int GetModelRoleForFullPaths();
	static int GetModelRoleForNames();

protected:
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void dragEnterEvent(QDragEnterEvent*) override;
	void dragMoveEvent(QDragMoveEvent*) override;
	void dropEvent(QDropEvent*) override;

private:
	QPoint m_dragStartPosition;
	std::weak_ptr<CurrentDirectoryFileOp> m_currentDirFileOp;

	static std::vector<QUrl> DecodeFileUris(const QString&);

	QString AggregateSelectionDataAsUriList() const;
	void pasteEvent();

	void CopyFileUrisIntoCurrentDir(const std::vector<QUrl>&);
};