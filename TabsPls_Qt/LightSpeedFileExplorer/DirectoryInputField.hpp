#pragma once

#include <optional>
#include <vector>

#include <QLineEdit>
#include <QString>

class DirectoryInputField : public QLineEdit {
    Q_OBJECT
  public:
    DirectoryInputField(QString initialDirectory);

  signals:
    void directoryChanged(QString);

  protected:
    void keyPressEvent(QKeyEvent*) override;
    bool focusNextPrevChild(bool) override { return false; } // In order to override behavior when pressing 'tab'

  private:
    QString m_currentDirectory;
    std::optional<std::vector<QString>> m_autoCompleteCandidates;

    void AutoCompleteCurrentPath(QKeyEvent* event);
};