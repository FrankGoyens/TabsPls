#pragma once

#include <optional>
#include <string>

namespace TabModel {
struct Tab {
    int index;
    std::string name;
};

bool operator==(const Tab& first, const Tab& second);

using Mnemonic = char;

struct TabLabel {
    std::optional<Mnemonic> mnemonic;
    std::string label;
};

bool operator==(const TabLabel& first, const TabLabel& second);

std::optional<Mnemonic> MnemonicFromIndex(int);
TabLabel LabelFromTabModel(const Tab&);

struct TabContainerItem {
    virtual ~TabContainerItem() = default;
    virtual Tab& GetTab() = 0;
};

template <typename TabContainer> void SwapTabs(TabContainer& tabs, int from, int to) {
    if (from >= tabs.size() || to >= tabs.size() || from < 0 || to < 0)
        return;

    const auto fromIt = std::begin(tabs) + from;
    const auto toIt = std::begin(tabs) + to;

    std::iter_swap(fromIt, toIt);

    (*toIt)->GetTab().index = to;
    (*fromIt)->GetTab().index = from;
}

template <typename TabContainer> void ReassignTabIndices(const TabContainer& tabs) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        tabs[tabIndex]->GetTab().index = tabIndex;
    }
}

template <typename TabContainer, typename TabLabelSetter>
void ReassignTabLabels(const TabContainer& tabs, const TabLabelSetter& setter) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        setter(tabIndex, LabelFromTabModel(tabs[tabIndex]->GetTab()));
    }
}
} // namespace TabModel
