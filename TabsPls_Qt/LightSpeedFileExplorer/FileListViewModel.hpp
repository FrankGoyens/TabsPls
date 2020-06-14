#include <QAbstractTableModel>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/FileSystemFilePath.hpp>

class FileListViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    FileListViewModel(const QString& initialDirectory);

    /*Qt table model implementation*/
    int rowCount(const QModelIndex & = QModelIndex()) const override
    {
        return 3;
    }

    int columnCount(const QModelIndex & = QModelIndex()) const override
    {
        return 1;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override
    {
        return { {Qt::DisplayRole, "display"} };
    }

    /*Custom*/

    //! \brief This will have no effect if the given directory does not exist
    void ChangeDirectory(const QString&);

private:
    std::vector<FileSystem::Directory> m_directoryEntries;
    std::vector<FileSystem::FilePath> m_fileEntries;
};