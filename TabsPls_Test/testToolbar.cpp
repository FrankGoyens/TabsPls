#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/Toolbar.hpp>

#include <TestPaths.h>

namespace ToolbarTests {

struct FakeActivator : TabsPlsPython::Toolbar::Activator {
    std::set<std::string> GetToolbarNames() const override { return toolbarNames; }

    TabsPlsPython::Toolbar::ActivationResult Activate(const TabsPlsPython::Toolbar::Toolbar& toolbar,
                                                      const TabsPlsPython::Toolbar::ToolbarItem& item) const override {
        activations.emplace_back(toolbar, item);
        return {};
    }

    std::set<std::string> toolbarNames;
    mutable std::vector<std::pair<TabsPlsPython::Toolbar::Toolbar, TabsPlsPython::Toolbar::ToolbarItem>> activations;
};

TEST(testToolbar, LoadToolbars) {
    TabsPlsPython::Toolbar::Init();

    const auto givenPluginsDir = FileSystem::Directory::FromPath(PLUGINS_TEST_INPUT);
    ASSERT_TRUE(givenPluginsDir);

    const auto createdToolbars = TabsPlsPython::Toolbar::LoadToolbars(*givenPluginsDir);

    ASSERT_EQ(1u, createdToolbars.size());
    ASSERT_EQ(3u, createdToolbars.front().GetItems().size());

    const TabsPlsPython::Toolbar::ToolbarItem expectedFirstToolbarItem{"abc", "/home/user"};
    const TabsPlsPython::Toolbar::ToolbarItem expectedSecondToolbarItem{"def", "/opt"};
    EXPECT_EQ(expectedFirstToolbarItem, createdToolbars.front().GetItems()[0]);
    EXPECT_EQ(expectedSecondToolbarItem, createdToolbars.front().GetItems()[1]);
}

TEST(testToolbar, Activate) {
    TabsPlsPython::Toolbar::Init();

    const auto givenPluginsDir = FileSystem::Directory::FromPath(PLUGINS_TEST_INPUT);
    ASSERT_TRUE(givenPluginsDir);

    TabsPlsPython::Toolbar::ToolbarItem givenFirstItem{"a", "b"}, givenSecondItem{"c", "d"};
    TabsPlsPython::Toolbar::Toolbar givenFirstToolbar{"firstToolbar", {givenFirstItem}},
        givenSecondToolbar{"secondToolbar", {givenSecondItem}};

    FakeActivator fakeActivator;
    fakeActivator.toolbarNames.insert("firstToolbar");
    TabsPlsPython::Toolbar::Activate(givenFirstToolbar, givenFirstItem, &fakeActivator);

    ASSERT_EQ(1u, fakeActivator.activations.size());
    EXPECT_EQ(givenFirstToolbar, fakeActivator.activations.front().first);
    EXPECT_EQ(givenFirstItem, fakeActivator.activations.front().second);

    fakeActivator.activations.clear();

    TabsPlsPython::Toolbar::Activate(givenFirstToolbar, givenSecondItem, &fakeActivator);
    ASSERT_EQ(0, fakeActivator.activations.size());

    TabsPlsPython::Toolbar::Activate(givenSecondToolbar, givenSecondItem, &fakeActivator);
    ASSERT_EQ(0, fakeActivator.activations.size()); // this toolbar is not present in result of 'GetNames()'
}

TEST(testToolbar, ActivateTestToolbar) {
    TabsPlsPython::Toolbar::Init();

    const auto givenPluginsDir = FileSystem::Directory::FromPath(PLUGINS_TEST_INPUT);
    ASSERT_TRUE(givenPluginsDir);

    const auto givenToolbars = TabsPlsPython::Toolbar::LoadToolbars(*givenPluginsDir);

    const auto testToolbarIt = std::find_if(givenToolbars.begin(), givenToolbars.end(),
                                            [](const auto& toolbar) { return toolbar.GetId() == "Toolbar_Tester"; });
    ASSERT_NE(testToolbarIt, givenToolbars.end());
    ASSERT_EQ(3u, testToolbarIt->GetItems().size());

    const auto createdFirstResult = TabsPlsPython::Toolbar::Activate(*testToolbarIt, testToolbarIt->GetItems()[0]);
    EXPECT_EQ("change_dir", createdFirstResult.desiredReaction);
    EXPECT_EQ("/home/user", createdFirstResult.parameter);

    const auto createdSecondResult = TabsPlsPython::Toolbar::Activate(*testToolbarIt, testToolbarIt->GetItems()[1]);
    EXPECT_EQ("change_dir", createdSecondResult.desiredReaction);
    EXPECT_EQ("/opt", createdSecondResult.parameter);

    const auto createdThirdResult = TabsPlsPython::Toolbar::Activate(*testToolbarIt, testToolbarIt->GetItems()[2]);
    EXPECT_EQ("message", createdThirdResult.desiredReaction);
    EXPECT_EQ("hello", createdThirdResult.parameter);
}
} // namespace ToolbarTests