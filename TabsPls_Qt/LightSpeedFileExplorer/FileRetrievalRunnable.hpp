#pragma once

#include <memory>

#include <QObject>
#include <QRunnable>
#include <QVector>

#include <TabsPlsCore/FileSystemDirectory.hpp>

#include "FileEntryModel.hpp"
#include "FileRetrievalByDispatch.hpp"

class FileRetrievalRunnable : public QObject, public QRunnable {
    Q_OBJECT
  public:
    FileRetrievalRunnable(FileSystem::Directory dir, const FileSystem::RawPath& basePath, const QIcon& fileIcon,
                          std::shared_ptr<const FileRetrievalByDispatch::DirectoryReadDispatcher> dispatcher)
        : m_dir(std::move(dir)), m_basePath(std::ref(basePath)), m_fileIcon(std::ref(fileIcon)),
          m_dispatcher(std::move(dispatcher)) {}

    void run() override;

  signals:
    void resultReady(QVector<FileEntryModel::ModelEntry>,
                     const FileRetrievalByDispatch::DirectoryReadDispatcher* usedDispatcher);

  private:
    FileSystem::Directory m_dir;
    std::reference_wrapper<const FileSystem::RawPath> m_basePath;
    std::reference_wrapper<const QIcon> m_fileIcon;
    std::shared_ptr<const FileRetrievalByDispatch::DirectoryReadDispatcher> m_dispatcher;
};
