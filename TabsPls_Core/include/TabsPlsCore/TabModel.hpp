#pragma once

#include <optional>
#include <string>

namespace TabModel {
struct Tab {
    int index;
    std::string name;
};

using Mnemonic = char;

struct TabLabel {
    std::optional<Mnemonic> mnemonic;
    std::string label;
    bool operator==(const TabLabel& other) const;
};

std::optional<Mnemonic> MnemonicFromIndex(int);
TabLabel LabelFromTabModel(const Tab&);

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
