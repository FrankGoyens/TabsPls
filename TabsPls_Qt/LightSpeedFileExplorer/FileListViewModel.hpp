#include <QAbstractTableModel>

#include <QIcon>
#include <QString>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

#include "DirectoryChanger.hpp"
#include "FileEntryModel.hpp"

class QStyle;
class QRunnable;

class FileListViewModel final : public QAbstractTableModel, public DirectoryChanger {
    Q_OBJECT

  public:
    FileListViewModel(QObject* parent, QStyle& styleProvider, const QString& initialDirectory);
    ~FileListViewModel() noexcept = default;

    /*Qt table model implementation*/
    int rowCount(const QModelIndex& = QModelIndex()) const override;

    int columnCount(const QModelIndex& = QModelIndex()) const override { return 3; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const override {
        return {{Qt::DisplayRole, "display"}, {Qt::DecorationRole, "icon_decoration"}, {Qt::UserRole, "full_paths"}};
    }

    /*TabsPls app*/

    void ChangeDirectory(const QString&) override;
    void RefreshDirectory(const QString&) override;
    std::optional<std::string> ClaimError() override;

  private slots:
    void RefreshIcon(QIcon, const QString& fullPath, QVariant reference);

  private:
    std::vector<FileSystem::Directory> m_directoryEntries;
    std::vector<FileEntryModel::FileEntry> m_fileEntries;

    std::vector<QString> m_displayName;
    std::vector<QString> m_displaySize;
    std::vector<QString> m_displayDateModified;
    std::vector<QString> m_fullPaths;
    std::vector<QIcon> m_icons;

    QStyle& m_styleProvider;

    std::optional<std::string> m_error;

    void FillModelDataCheckingForRoot(const QString& dir);
    void FillModelData();
    void FillIcons();

    QRunnable* MakeIconRetrievalThread(const std::wstring& fullPathStdString, int index);

    std::optional<std::reference_wrapper<const std::vector<QString>>> GetDisplayDataForColumn(int column) const;
    [[nodiscard]] bool ShouldProceedWithRename(int row, int col, const QString& value);
};