#include <gtest/gtest.h>

#include <TabsPlsCore/FileSystemDirectory.hpp>
#include <TabsPlsCore/Toolbar.hpp>

#include <TestPaths.h>

using namespace TabsPlsPython::Toolbar;

namespace ToolbarTests {

struct FakeActivator : Activator {
    std::set<std::string> GetToolbarNames() const override { return toolbarNames; }

    ActivationResult Activate(const Toolbar& toolbar, const ToolbarItem& item,
                              ::ActivationMethod activationMethod) const override {
        activations.emplace_back(toolbar, item, activationMethod);
        return {};
    }

    std::set<std::string> toolbarNames;
    mutable std::vector<std::tuple<Toolbar, ToolbarItem, ActivationMethod>> activations;
};

TEST(testToolbar, LoadToolbars) {
    Init();

    const auto givenPluginsDir = FileSystem::Directory::FromPath(PLUGINS_TEST_INPUT);
    ASSERT_TRUE(givenPluginsDir);

    const auto createdToolbars = LoadToolbars(*givenPluginsDir);

    ASSERT_EQ(1u, createdToolbars.size());
    ASSERT_EQ(3u, createdToolbars.front().GetItems().size());

    const ToolbarItem expectedFirstToolbarItem{"abc", "/home/user"};
    const ToolbarItem expectedSecondToolbarItem{"def", "/opt"};
    EXPECT_EQ(expectedFirstToolbarItem, createdToolbars.front().GetItems()[0]);
    EXPECT_EQ(expectedSecondToolbarItem, createdToolbars.front().GetItems()[1]);
}

TEST(testToolbar, Activate) {
    Init();

    const auto givenPluginsDir = FileSystem::Directory::FromPath(PLUGINS_TEST_INPUT);
    ASSERT_TRUE(givenPluginsDir);

    ToolbarItem givenFirstItem{"a", "b"}, givenSecondItem{"c", "d"};
    Toolbar givenFirstToolbar{"firstToolbar", {givenFirstItem}}, givenSecondToolbar{"secondToolbar", {givenSecondItem}};

    FakeActivator fakeActivator;
    fakeActivator.toolbarNames.insert("firstToolbar");
    Activate(givenFirstToolbar, givenFirstItem, ActivationMethod::Regular, &fakeActivator);

    ASSERT_EQ(1u, fakeActivator.activations.size());
    EXPECT_EQ(givenFirstToolbar, std::get<0>(fakeActivator.activations.front()));
    EXPECT_EQ(givenFirstItem, std::get<1>(fakeActivator.activations.front()));
    EXPECT_EQ(ActivationMethod::Regular, std::get<2>(fakeActivator.activations.front()));

    fakeActivator.activations.clear();

    Activate(givenFirstToolbar, givenFirstItem, ActivationMethod::Alternative, &fakeActivator);

    ASSERT_EQ(1u, fakeActivator.activations.size());
    EXPECT_EQ(givenFirstToolbar, std::get<0>(fakeActivator.activations.front()));
    EXPECT_EQ(givenFirstItem, std::get<1>(fakeActivator.activations.front()));
    EXPECT_EQ(ActivationMethod::Alternative, std::get<2>(fakeActivator.activations.front()));

    fakeActivator.activations.clear();

    Activate(givenFirstToolbar, givenSecondItem, ActivationMethod::Regular, &fakeActivator);
    ASSERT_EQ(0, fakeActivator.activations.size());

    Activate(givenSecondToolbar, givenSecondItem, ActivationMethod::Regular, &fakeActivator);
    ASSERT_EQ(0, fakeActivator.activations.size()); // this toolbar is not present in result of 'GetNames()'
}

TEST(testToolbar, ActivateTestToolbar) {
    Init();

    const auto givenPluginsDir = FileSystem::Directory::FromPath(PLUGINS_TEST_INPUT);
    ASSERT_TRUE(givenPluginsDir);

    const auto givenToolbars = LoadToolbars(*givenPluginsDir);

    const auto testToolbarIt = std::find_if(givenToolbars.begin(), givenToolbars.end(),
                                            [](const auto& toolbar) { return toolbar.GetId() == "Toolbar_Tester"; });
    ASSERT_NE(testToolbarIt, givenToolbars.end());
    ASSERT_EQ(3u, testToolbarIt->GetItems().size());

    const auto createdFirstResult = Activate(*testToolbarIt, testToolbarIt->GetItems()[0]);
    EXPECT_EQ("change_dir", createdFirstResult.desiredReaction);
    EXPECT_EQ("/home/user", createdFirstResult.parameter);

    const auto createdSecondResult = Activate(*testToolbarIt, testToolbarIt->GetItems()[1]);
    EXPECT_EQ("change_dir", createdSecondResult.desiredReaction);
    EXPECT_EQ("/opt", createdSecondResult.parameter);

    const auto createdThirdResult = Activate(*testToolbarIt, testToolbarIt->GetItems()[2]);
    EXPECT_EQ("message", createdThirdResult.desiredReaction);
    EXPECT_EQ("hello", createdThirdResult.parameter);
}
} // namespace ToolbarTests