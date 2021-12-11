#include <TabsPlsCore/TabModel.hpp>

namespace TabModel {

std::optional<Mnemonic> MnemonicFromIndex(int index) {
    if (index > 9)
        return {};

    if (index == 9)
        return '0';

    return std::to_string(index + 1)[0];
}
TabLabel LabelFromTabModel(const Tab& tabModel) { return {MnemonicFromIndex(tabModel.index), tabModel.name}; }
} // namespace TabModel
