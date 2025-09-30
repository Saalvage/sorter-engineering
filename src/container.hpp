#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace ae {

class sorter;

class container {
    friend class sorter;

public:
    using element_type = std::uint64_t;

    explicit container(std::span<const element_type> data, std::size_t num_threads);

private:
    std::vector<std::vector<element_type>> segments;

public:
    [[nodiscard]] std::span<element_type> segment(int idx);

    [[nodiscard]] auto to_view(int start, int end) {
        return std::views::join(std::ranges::subrange(segments.begin() + start, segments.begin() + end));
    }

    [[nodiscard]] auto to_view() const {
        return std::views::join(segments);
    }
};

}  // namespace ae
