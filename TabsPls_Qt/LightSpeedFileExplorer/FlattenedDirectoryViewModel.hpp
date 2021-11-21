#pragma once

#include <TabsPlsCore/FileSystemFilePath.hpp>

#include <QAbstractTableModel>
#include <QIcon>

class FlattenedDirectoryViewModel final : public QAbstractTableModel {
    Q_OBJECT

  public:
    FlattenedDirectoryViewModel() = default;
    ~FlattenedDirectoryViewModel() = default;

    /*Qt table model implementation*/
    int rowCount(const QModelIndex& = QModelIndex()) const override;

    int columnCount(const QModelIndex& = QModelIndex()) const override { return 3; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role) const override;

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

    struct ModelEntry {
        QString displayName;
        QString displaySize;
        QString displayDateModified;
        QString fullPath;
        QIcon icon;
    };

  private:
    struct FileEntry {
        FileSystem::FilePath filePath;
        std::time_t lastModificationDate;
        std::uintmax_t size;
    };

    std::vector<FileEntry> m_fileEntries;

    std::vector<ModelEntry> m_modelEntries;
};