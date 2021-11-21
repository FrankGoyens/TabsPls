#pragma once

#include <TabsPlsCore/FileSystemFilePath.hpp>

#include <QAbstractTableModel>
#include <QIcon>

#include "DirectoryChanger.hpp"

class FlattenedDirectoryViewModel final : public QAbstractTableModel, public DirectoryChanger {
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

    void ChangeDirectory(const QString&) override;
    void RefreshDirectory(const QString&) override;
    std::optional<std::string> ClaimError() override;

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