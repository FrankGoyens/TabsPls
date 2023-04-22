#include "PluginProvisionedToolbar.hpp"

#include <QMessageBox>
#include <QToolButton>

#include "MouseWheelClickEventFilter.hpp"

static QString MakeMessageHeader(const TabsPlsPython::Toolbar::Toolbar& toolbar) {
    return QString(QObject::tr("Message from %1 toolbar").arg(QString::fromStdString(toolbar.GetId())));
}

PluginProvisionedToolbar::PluginProvisionedToolbar(TabsPlsPython::Toolbar::Toolbar toolbarModel)
    : m_toolbarModel(std::move(toolbarModel)) {

    for (const auto& item : m_toolbarModel.GetItems()) {
        auto* action = addAction(QString::fromStdString(item.displayName));
        connect(action, &QAction::triggered, [this, item] {
            HandleResultingRequest(TabsPlsPython::Toolbar::Activate(m_toolbarModel, item,
                                                                    TabsPlsPython::Toolbar::ActivationMethod::Regular));
        });
        auto* wheelEventFilter = new MouseWheelClickEventFilter();
        connect(wheelEventFilter, &MouseWheelClickEventFilter::MouseWheelClicked, [this, item] {
            HandleResultingRequest(TabsPlsPython::Toolbar::Activate(
                m_toolbarModel, item, TabsPlsPython::Toolbar::ActivationMethod::Alternative));
        });
        widgetForAction(action)->installEventFilter(wheelEventFilter);
    }
}

void PluginProvisionedToolbar::HandleResultingRequest(const TabsPlsPython::Toolbar::ActivationResult& result) {
    if (result.desiredReaction == "message")
        QMessageBox::information(this, MakeMessageHeader(m_toolbarModel), QString::fromStdString(result.parameter));
    else if (result.desiredReaction == "change_dir")
        emit RequestChangeDirectory(QString::fromStdString(result.parameter));
    else if (result.desiredReaction == "open_dir_in_tab")
        emit RequestOpenDirectoryInNewTab(QString::fromStdString(result.parameter));
}
