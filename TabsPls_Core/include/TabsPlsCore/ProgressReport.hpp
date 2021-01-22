#pragma once

struct ProgressReport {
    virtual void UpdateValue(int) = 0;
    virtual void SetMinimum(int) = 0;
    virtual void SetMaximum(int) = 0;
};