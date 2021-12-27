#include "FileRetrievalRunnable.hpp"

#include "FileRetrievalByDispatch.hpp"

void FileRetrievalRunnable::run() {
    try {
        const auto modelEntries = FileRetrievalByDispatch::AsModelEntries(
            FileRetrievalByDispatch::RetrieveFiles(m_dir, *m_dispatcher), m_basePath.get(), m_fileIcon.get());

        const FileRetrievalRunnableContainer::NameSortedModelSet sortedModelEntries(
            modelEntries.begin(), modelEntries.end(), &FileEntryModel::ModelEntryDisplayNameSortingPredicate);

        emit resultReady(sortedModelEntries, m_dispatcher.get());
    } catch (const std::exception&) {
        // TODO logging
    }
}