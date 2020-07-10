#include <QAbstractTableModel>

#include <QString>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

class FileListViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    FileListViewModel(const QString& initialDirectory);

    /*Qt table model implementation*/
    int rowCount(const QModelIndex & = QModelIndex()) const override;

    int columnCount(const QModelIndex & = QModelIndex()) const override
    {
        return 1;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const override
    {
        return { {Qt::DisplayRole, "display"}, {Qt::UserRole, "full_paths" } };
    }

    /*TabsPls app*/

    //! \brief This will have no effect if the given directory does not exist
    void ChangeDirectory(const QString&);

    /*! \brief This will have no effect if the given directory does not exist
    * 
    * This should be called instead of 'ChangeDirectory' when the current directory did not change.
    */
    void RefreshDirectory(const QString&);

    std::optional<std::string> ClaimError();

private:
    std::vector<FileSystem::Directory> m_directoryEntries;
    std::vector<FileSystem::FilePath> m_fileEntries;

    std::vector<QString> m_display;
    std::vector<QString> m_fullPaths;

    std::optional<std::string> m_error;

    void FillModelDataCheckingForRoot(const QString& dir);
    void FillModelData();
};