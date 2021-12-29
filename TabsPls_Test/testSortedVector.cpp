#include <gtest/gtest.h>

#include <TabsPlsCore/SortedVector.hpp>

TEST(testSortedVector, Constructor) {
    const SortedVector<int, std::less<int>> givenSortedVector({4, 6, 2, 1, 8, 9}, std::less<int>{});
    const std::vector<int> expectedVector{1, 2, 4, 6, 8, 9};
    EXPECT_EQ(expectedVector, givenSortedVector.get());
}

static void ExpectElementAtIndex(const SortedVector<int, std::less<int>>& container, int element, int index) {
    const auto it = container.find(1);
    ASSERT_NE(container.get().end(), it);
    EXPECT_EQ(1, *it);
    EXPECT_EQ(0, it - container.get().begin());
}

TEST(testSortedVector, find) {
    const SortedVector<int, std::less<int>> givenSortedVector({4, 6, 2, 1, 8, 9}, std::less<int>{});

    ExpectElementAtIndex(givenSortedVector, 1, 0);
    ExpectElementAtIndex(givenSortedVector, 2, 1);
    ExpectElementAtIndex(givenSortedVector, 4, 2);
    ExpectElementAtIndex(givenSortedVector, 6, 3);
    ExpectElementAtIndex(givenSortedVector, 8, 4);
    ExpectElementAtIndex(givenSortedVector, 9, 5);

    EXPECT_EQ(givenSortedVector.get().end(), givenSortedVector.find(123));
    EXPECT_EQ(givenSortedVector.get().end(), givenSortedVector.find(-987));
}

TEST(testSortedVector, lower_bound) {
    const SortedVector<int, std::less<int>> givenSortedVector({4, 6, 2, 1, 8, 9}, std::less<int>{});

    EXPECT_EQ(givenSortedVector.get().begin(), givenSortedVector.lower_bound(-987));
    EXPECT_EQ(givenSortedVector.get().begin() + 2, givenSortedVector.lower_bound(4));
    EXPECT_EQ(givenSortedVector.get().begin() + 3, givenSortedVector.lower_bound(5));
    EXPECT_EQ(givenSortedVector.get().end(), givenSortedVector.lower_bound(123));
}

TEST(testSortedVector, insert) {
    SortedVector<int, std::less<int>> givenSortedVector({1, 5, 6, 7, 9}, std::less<int>{});
    const SortedVector<int, std::less<int>> givenOtherSortedVector({2, 3, 4, 8}, std::less<int>{});

    givenSortedVector.insert(givenOtherSortedVector);

    const std::vector<int> expectedVector{1, 2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(expectedVector, givenSortedVector.get());
}

TEST(testSortedVector, insert_into_empty) {
    SortedVector<int, std::less<int>> givenSortedVector({}, std::less<int>{});
    const SortedVector<int, std::less<int>> givenOtherSortedVector({2, 3, 4, 8}, std::less<int>{});

    givenSortedVector.insert(givenOtherSortedVector);

    const std::vector<int> expectedVector{2, 3, 4, 8};
    EXPECT_EQ(expectedVector, givenSortedVector.get());
}

TEST(testSortedVector, insert_empty_into_empty) {
    SortedVector<int, std::less<int>> givenSortedVector({}, std::less<int>{});
    const SortedVector<int, std::less<int>> givenOtherSortedVector({}, std::less<int>{});

    givenSortedVector.insert(givenOtherSortedVector);

    const std::vector<int> expectedVector{};
    EXPECT_EQ(expectedVector, givenSortedVector.get());
}