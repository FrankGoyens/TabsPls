#pragma once

#include <functional>

#include <QObject>

#include <TabsPlsCore/Send2Trash.hpp>

#include "ExplicitStub.hpp"
#include "TabsPlsLog.hpp"

class QObjectRecycleExceptionHandler : public QObject {
    Q_OBJECT

  public:
    template <typename Result> Result DoWithRecycleExceptionHandling(std::function<Result()> func, Result fallback) {
        try {
            return func();
        } catch (const TabsPlsPython::Send2Trash::ModuleNotFoundException&) {
            emit ModuleNotFound(QObject::tr("Recycle item"), QObject::tr("The send2trash module could not be found, ") +
                                                                 QObject::tr("please reinstall this program."));
        } catch (const TabsPlsPython::Send2Trash::Exception&) {
            emit GenericError(QObject::tr("Recycle item"), QObject::tr("Unknown eror"));
        } catch (const ExplicitStubException&) {
            // The component is supposedly available, but we're still somehow calling the stubbed implementation.
            TabsPlsLog_Debug("An explicitly stubbed component was called: Send2Trash");

            // let's not bother the user with this information
            emit ExplicitStubError(QObject::tr("Recycle item"), QObject::tr("Unknown error"));
        }
        return fallback;
    }
  signals:
    void ModuleNotFound(QString title, QString message);
    void GenericError(QString title, QString message);
    void ExplicitStubError(QString title, QString message);
};