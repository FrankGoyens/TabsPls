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

template <typename TabContainer> void SwapTabs(TabContainer& tabs, int from, int to) {
    if (from >= tabs.size() || to >= tabs.size())
        return;

    const auto fromIt = std::begin(tabs) + from;
    const auto toIt = std::begin(tabs) + to;

    using std::swap;
    swap(**fromIt, **toIt);

    (*toIt)->index = to;
    (*fromIt)->index = from;
}

template <typename TabContainer> void ReassignTabIndices(const TabContainer& tabs) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        tabs[tabIndex]->index = tabIndex;
    }
}

template <typename TabContainer, typename TabLabelSetter>
void ReassignTabLabels(const TabContainer& tabs, const TabLabelSetter& setter) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        setter(tabIndex, LabelFromTabModel(*tabs[tabIndex]));
    }
}
} // namespace TabModel
