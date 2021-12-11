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
};

std::optional<Mnemonic> MnemonicFromIndex(int);
TabLabel LabelFromTabModel(const Tab&);

template <typename TabPtr> inline void SetIndex(TabPtr& tab, int index) { tab->index = index; }

template <> inline void SetIndex(Tab& tab, int index) { tab.index = index; }

template <typename TabContainer> void ReassignTabIndices(const TabContainer& tabs) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        SetIndex(tabs[tabIndex], tabIndex);
    }
}

template <typename TabPtr> inline const Tab& Deref(const TabPtr& tab) { return *tab; }

template <> inline const Tab& Deref(const Tab& tab) { return tab; }

template <typename TabContainer, typename TabLabelSetter>
void ReassignTabLabels(const TabContainer& tabs, const TabLabelSetter& setter) {
    for (int tabIndex = 0; tabIndex < tabs.size(); ++tabIndex) {
        setter(tabIndex, LabelFromTabModel(Deref(tabs[tabIndex])));
    }
}
} // namespace TabModel
