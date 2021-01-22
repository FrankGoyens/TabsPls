#pragma once

#include <QObject>

#include <TabsPlsCore/ProgressReport.hpp>

class QObjectProgressReport : public QObject, public ProgressReport {
    Q_OBJECT
  public:
    void UpdateValue(int) override;
    void SetMinimum(int) override;
    void SetMaximum(int) override;

  signals:
    void Updated(int);
    void MinimumSet(int);
    void MaximumSet(int);
};