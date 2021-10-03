#include <QAbstractTableModel>

#include <QIcon>
#include <QString>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

class QStyle;

class FileListViewModel final : public QAbstractTableModel {
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

    //! \brief This will have no effect if the given directory does not exist
    void ChangeDirectory(const QString&);

    /*! \brief This will have no effect if the given directory does not exist
     *
     * This should be called instead of 'ChangeDirectory' when the current
     * directory did not change.
     */
    void RefreshDirectory(const QString&);

    std::optional<std::string> ClaimError();

    struct FileEntry {
        FileSystem::FilePath filePath;
        std::time_t lastModificationDate;
        std::uintmax_t size;
    };

  private slots:
    void RefreshIcon(QIcon, const QString& fullPath, int index);

  private:
    std::vector<FileSystem::Directory> m_directoryEntries;
    std::vector<FileEntry> m_fileEntries;

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

    void StartIconRetrievalThread(const std::wstring& fullPathStdString, int index);

    std::optional<std::reference_wrapper<const std::vector<QString>>> GetDisplayDataForColumn(int column) const;
};