#pragma once

#include <memory>
#include <optional>

#include <QTableView>

class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QContextMenuEvent;
class CurrentDirectoryFileOp;
class QObjectRecycleExceptionHandler;

class FileListTableView : public QTableView {
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

    void commitData(QWidget* editor) override;

    void contextMenuEvent(QContextMenuEvent*) override;

  private slots:
    void ShowCriticalWorkerError(QString title, QString message);

  private:
    QPoint m_dragStartPosition;
    std::weak_ptr<CurrentDirectoryFileOp> m_currentDirFileOp;

    QString AggregateSelectionDataAsUriList() const;
    QStringList AggregateSelectionDataAsLocalFileList() const;
    void NotifyModelOfChange();
    void pasteEvent();
    void PerformMimeDataActionOnIncomingFiles(const QMimeData&, const std::vector<QUrl>&);
    void AskRecycleSelectedFiles(CurrentDirectoryFileOp&);
    void ConnectRecyclingErrorSignals(const QObjectRecycleExceptionHandler&);
    void AskPermanentlyDeleteSelectedFiles();

    void CopyFileUrisIntoCurrentDir(const std::vector<QUrl>&);
    void MoveFileUrisIntoCurrentDir(const std::vector<QUrl>&);

    template <typename OpFunction>
    void DoFileOpWhileShowingProgress(const std::vector<QUrl>& urls, const QString& duringOpTitle,
                                      const QString& opDoneWithErrortitle, OpFunction opFunction);

    std::optional<std::shared_ptr<CurrentDirectoryFileOp>> ValidateFileOpPrecondition(const std::vector<QUrl>&);

    void CompleteFileOp(const QString& opName, const QStringList& result);
};
