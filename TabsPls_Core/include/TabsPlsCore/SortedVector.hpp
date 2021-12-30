#pragma once

#include <algorithm>
#include <vector>

template <typename T, typename Comp> class SortedVector {
  public:
    using key_compare = Comp;

    using container_impl = std::vector<T>;

    SortedVector() = default;

    SortedVector(std::vector<T> initialItems, Comp comp) : m_comp(std::move(comp)) {
        std::sort(initialItems.begin(), initialItems.end(), m_comp);
        m_items = std::move(initialItems);
    }

    void insert(SortedVector other) {
        if (other.get().empty())
            return;

        auto [lower_bound_index, upper_bound_index] =
            lower_and_upper_bound_index(other.get().front(), other.get().back());

        m_items.insert(m_items.begin() + lower_bound_index, std::move(other.get().front()));
        shift_bounds(lower_bound_index, upper_bound_index);

        for (auto it = other.get().begin() + 1; it != other.get().end(); ++it) {
            const auto current_lower_bound =
                std::lower_bound(m_items.begin() + lower_bound_index, m_items.begin() + upper_bound_index, *it, m_comp);
            m_items.insert(current_lower_bound, std::move(*it));
            shift_bounds(lower_bound_index, upper_bound_index);
        }
    }

    static bool items_equal(const T& first, const T& second, const Comp& comp) {
        return !comp(first, second) && !comp(second, first);
    }

    auto find(const T& item) const {
        const auto lower_bound_it = lower_bound(item);
        if (lower_bound_it == m_items.end())
            return m_items.end();
        if (!items_equal(item, *lower_bound_it, m_comp))
            return m_items.end();
        return lower_bound_it;
    }

    auto lower_bound(const T& item) const { return std::lower_bound(m_items.begin(), m_items.end(), item, m_comp); }

    void clear() { m_items.clear(); }

    const std::vector<T>& get() const { return m_items; }

  private:
    std::vector<T> m_items;
    Comp m_comp;

    auto lower_and_upper_bound_index(const T& first, const T& second) {
        const auto lower_bound_index = lower_bound(first) - m_items.begin();
        const auto upper_bound_it =
            std::upper_bound(m_items.begin() + lower_bound_index, m_items.end(), second, m_comp);
        return std::make_pair(lower_bound_index, upper_bound_it - m_items.begin());
    }

    template <typename SizeT> static void shift_bounds(SizeT& lower, SizeT& upper) {
        ++lower;
        ++upper;
    }
};
