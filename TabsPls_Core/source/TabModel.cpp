#include <TabsPlsCore/TabModel.hpp>

namespace TabModel {
bool operator==(const Tab& first, const Tab& second) {
    return first.index == second.index && first.name == second.name;
}
bool operator==(const TabLabel& first, const TabLabel& second) {
    return first.mnemonic == second.mnemonic && first.label == second.label;
}

std::optional<Mnemonic> MnemonicFromIndex(int index) {
    if (index > 9 || index < 0)
        return {};

    if (index == 9)
        return '0';

    return std::to_string(index + 1)[0];
}

TabLabel LabelFromTabModel(const Tab& tabModel) { return {MnemonicFromIndex(tabModel.index), tabModel.name}; }

} // namespace TabModel
