#pragma once

#include <string>

#include <QRunnable>

#include <AssociatedIconProvider.hpp>

class IconRetrievalRunnable : public QObject, public QRunnable {
    Q_OBJECT
  public:
    IconRetrievalRunnable(const FileSystem::RawPath& path, int index) : m_path(path), m_index(index) {}

    void run() override {
        if (AssociatedIconProvider::ComponentIsAvailable()) {
            if (const auto associatedIcon = AssociatedIconProvider::Get().FromPath(m_path)) {
                emit resultReady(*associatedIcon, QString::fromStdWString(m_path), m_index);
            }
        }
    }

  signals:
    void resultReady(const QIcon& icon, const QString& path, int index);

  private:
    FileSystem::RawPath m_path;
    int m_index;
};