#pragma once

#include <string>

#include <QRunnable>
#include <QVariant>

#include <AssociatedIconProvider.hpp>

class IconRetrievalRunnable : public QObject, public QRunnable {
    Q_OBJECT
  public:
    IconRetrievalRunnable(const FileSystem::RawPath& path, QVariant reference) : m_path(path), m_reference(reference) {}

    void run() override {
        if (AssociatedIconProvider::ComponentIsAvailable()) {
            AssociatedIconProvider::InitThread();
            if (const auto associatedIcon = AssociatedIconProvider::Get().FromPath(m_path)) {
                emit resultReady(*associatedIcon, QString::fromStdWString(m_path), m_reference);
            }
        }
    }

  signals:
    void resultReady(const QIcon& icon, const QString& path, QVariant reference);

  private:
    FileSystem::RawPath m_path;
    QVariant m_reference;
};