#include "FileRetrievalRunnable.hpp"

#include "FileRetrievalByDispatch.hpp"

void FileRetrievalRunnable::run() {
    const auto modelEntries = FileRetrievalByDispatch::AsModelEntries(
        FileRetrievalByDispatch::RetrieveFiles(m_dir, *m_dispatcher), m_basePath.get(), m_fileIcon.get());
    emit resultReady(QVector<FileEntryModel::ModelEntry>::fromStdVector(std::move(modelEntries)), m_dispatcher.get());
}
