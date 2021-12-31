#pragma once

#include <memory>

#include <QMetaType>
#include <QObject>
#include <QRunnable>
#include <QVector>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/SortedVector.hpp>

#include "FileEntryModel.hpp"
#include "FileRetrievalByDispatch.hpp"

namespace FileRetrievalRunnableContainer {
using NameSortedModelSet =
    SortedVector<FileEntryModel::ModelEntry, decltype(&FileEntryModel::ModelEntryDepthSortingPredicate)>;
}

class FileRetrievalRunnable : public QObject, public QRunnable {
    Q_OBJECT
  public:
    FileRetrievalRunnable(FileSystem::Directory dir, const FileSystem::RawPath& basePath, QIcon fileIcon,
                          std::shared_ptr<const FileRetrievalByDispatch::DirectoryReadDispatcher> dispatcher)
        : m_dir(std::move(dir)), m_basePath(std::ref(basePath)), m_fileIcon(std::move(fileIcon)),
          m_dispatcher(std::move(dispatcher)) {}

    void run() override;

  signals:
    void resultReady(FileRetrievalRunnableContainer::NameSortedModelSet,
                     const FileRetrievalByDispatch::DirectoryReadDispatcher* usedDispatcher);

  private:
    FileSystem::Directory m_dir;
    std::reference_wrapper<const FileSystem::RawPath> m_basePath;
    QIcon m_fileIcon;
    std::shared_ptr<const FileRetrievalByDispatch::DirectoryReadDispatcher> m_dispatcher;
};

Q_DECLARE_METATYPE(FileRetrievalRunnableContainer::NameSortedModelSet)