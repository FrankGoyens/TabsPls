#pragma once

#include <string>

#include <QThread>

#include <AssociatedIconProvider.hpp>

class IconRetrievalThread : public QThread {
  public:
    IconRetrievalThread(const std::wstring& path, int index) : m_path(path), m_index(index) {}

    Q_OBJECT
    void run() override {
        if (const auto associatedIcon = AssociatedIconProvider::Get().FromPath(m_path)) {
            emit resultReady(*associatedIcon, QString::fromStdWString(m_path), m_index);
        }
    }

  signals:
    void resultReady(const QIcon& icon, const QString& path, int index);

  private:
    std::wstring m_path;
    int m_index;
};