#include <gtest/gtest.h>

#include <TabsPlsCore/TabModel.hpp>

namespace TabModel {

static std::ostream& operator<<(std::ostream& os, const TabLabel& label) {
    const auto mnemonicPrint = label.mnemonic ? std::string(&*label.mnemonic, 1) : std::string("<none>");
    os << "TabLabel: mnemonic: " << mnemonicPrint << " label: " << label.label;
    return os;
}

} // namespace TabModel

namespace {
struct FakeTabContainerItem : TabModel::TabContainerItem {
    FakeTabContainerItem(TabModel::Tab tab) : tab(std::move(tab)) {}
    TabModel::Tab& GetTab() override { return tab; }
    TabModel::Tab tab;
};
} // namespace

TEST(testTabModel, MnemonicFromIndex) {
    ASSERT_TRUE(TabModel::MnemonicFromIndex(0));
    EXPECT_EQ('1', *TabModel::MnemonicFromIndex(0));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(1));
    EXPECT_EQ('2', *TabModel::MnemonicFromIndex(1));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(2));
    EXPECT_EQ('3', *TabModel::MnemonicFromIndex(2));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(3));
    EXPECT_EQ('4', *TabModel::MnemonicFromIndex(3));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(4));
    EXPECT_EQ('5', *TabModel::MnemonicFromIndex(4));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(5));
    EXPECT_EQ('6', *TabModel::MnemonicFromIndex(5));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(6));
    EXPECT_EQ('7', *TabModel::MnemonicFromIndex(6));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(7));
    EXPECT_EQ('8', *TabModel::MnemonicFromIndex(7));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(8));
    EXPECT_EQ('9', *TabModel::MnemonicFromIndex(8));

    ASSERT_TRUE(TabModel::MnemonicFromIndex(9));
    EXPECT_EQ('0', *TabModel::MnemonicFromIndex(9));

    ASSERT_FALSE(TabModel::MnemonicFromIndex(10));
    ASSERT_FALSE(TabModel::MnemonicFromIndex(-1));
}

TEST(testTabModel, LabelFromTabModel) {
    const TabModel::TabLabel expectedLabel{'1', "nice_tab"};
    EXPECT_EQ(expectedLabel, TabModel::LabelFromTabModel(TabModel::Tab{0, "nice_tab"}));
}

TEST(testTabModel, ReassignTabIndices) {
    const std::vector givenTabContainer = {std::make_shared<FakeTabContainerItem>(TabModel::Tab{-99, "first_tab"}),
                                           std::make_shared<FakeTabContainerItem>(TabModel::Tab{-88, "second_tab"})};

    TabModel::ReassignTabIndices(givenTabContainer);

    EXPECT_EQ(0, givenTabContainer[0]->GetTab().index);
    EXPECT_EQ(1, givenTabContainer[1]->GetTab().index);
}

TEST(testTabModel, ReassignTabLabels) {
    std::pair<char, std::string> givenFirstTabViewer = {'x', "a_tab"};
    std::pair<char, std::string> givenSecondTabViewer = {'y', "another_tab"};

    const std::vector givenTabContainer = {std::make_shared<FakeTabContainerItem>(TabModel::Tab{0, "first_tab"}),
                                           std::make_shared<FakeTabContainerItem>(TabModel::Tab{1, "second_tab"})};

    TabModel::ReassignTabLabels(givenTabContainer, [&](int index, const auto& tabLabel) {
        ASSERT_TRUE(tabLabel.mnemonic);
        if (index == 0)
            givenFirstTabViewer = std::make_pair(*tabLabel.mnemonic, tabLabel.label);
        if (index == 1)
            givenSecondTabViewer = std::make_pair(*tabLabel.mnemonic, tabLabel.label);
    });

    EXPECT_EQ(std::make_pair('1', std::string("first_tab")), givenFirstTabViewer);
    EXPECT_EQ(std::make_pair('2', std::string("second_tab")), givenSecondTabViewer);
}

TEST(testTabModel, SwapTabs) {
    std::vector givenTabContainer = {std::make_shared<FakeTabContainerItem>(TabModel::Tab{0, "first_tab"}),
                                     std::make_shared<FakeTabContainerItem>(TabModel::Tab{1, "second_tab"})};

    TabModel::SwapTabs(givenTabContainer, 0, 1);

    const TabModel::Tab expectedFirst{0, std::string("second_tab")};
    const TabModel::Tab expectedSecond{1, std::string("first_tab")};
    EXPECT_EQ(expectedFirst, givenTabContainer[0]->GetTab());
    EXPECT_EQ(expectedSecond, givenTabContainer[1]->GetTab());

    EXPECT_NO_THROW(TabModel::SwapTabs(givenTabContainer, 1000, 878));
}