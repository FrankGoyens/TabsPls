#include <TabsPlsCore/TabModel.hpp>

namespace TabModel {

bool TabLabel::operator==(const TabLabel& other) const { return mnemonic == other.mnemonic && label == other.label; }

std::optional<Mnemonic> MnemonicFromIndex(int index) {
    if (index > 9 || index < 0)
        return {};

    if (index == 9)
        return '0';

    return std::to_string(index + 1)[0];
}

TabLabel LabelFromTabModel(const Tab& tabModel) { return {MnemonicFromIndex(tabModel.index), tabModel.name}; }

} // namespace TabModel
