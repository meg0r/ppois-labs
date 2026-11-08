#pragma once
#include <iterator>
#include <functional>
#include <utility>
#include <cstddef>

namespace lab4 {

namespace detail {

template <typename RandomIt, typename Compare>
void siftDown(RandomIt first, std::ptrdiff_t start, std::ptrdiff_t end, Compare comp) {
    std::ptrdiff_t root = start;
    while (2 * root + 1 <= end) {
        std::ptrdiff_t child = 2 * root + 1;
        std::ptrdiff_t swap_idx = root;

        if (comp(*(first + swap_idx), *(first + child))) {
            swap_idx = child;
        }
        if (child + 1 <= end && comp(*(first + swap_idx), *(first + child + 1))) {
            swap_idx = child + 1;
        }
        if (swap_idx == root) {
            return;
        } else {
            std::iter_swap(first + root, first + swap_idx);
            root = swap_idx;
        }
    }
}

template <typename RandomIt, typename Compare>
void stoogeSortImpl(RandomIt first, std::ptrdiff_t i, std::ptrdiff_t j, Compare comp) {
    if (comp(*(first + j), *(first + i))) {
        std::iter_swap(first + i, first + j);
    }
    if (j - i + 1 > 2) {
        std::ptrdiff_t t = (j - i + 1) / 3;
        stoogeSortImpl(first, i, j - t, comp);
        stoogeSortImpl(first, i + t, j, comp);
        stoogeSortImpl(first, i, j - t, comp);
    }
}

} // namespace detail

// 1. Heapsort for iterator ranges
template <typename RandomIt, typename Compare = std::less<>>
void heapSort(RandomIt first, RandomIt last, Compare comp = Compare()) {
    std::ptrdiff_t count = std::distance(first, last);
    if (count <= 1) return;

    for (std::ptrdiff_t start = (count - 2) / 2; start >= 0; --start) {
        detail::siftDown(first, start, count - 1, comp);
    }

    for (std::ptrdiff_t end = count - 1; end > 0; --end) {
        std::iter_swap(first, first + end);
        detail::siftDown(first, 0, end - 1, comp);
    }
}

// Heapsort overload for standard containers (std::vector, etc.)
template <typename Container, typename Compare = std::less<>>
void heapSort(Container& container, Compare comp = Compare()) {
    heapSort(std::begin(container), std::end(container), comp);
}

// Heapsort overload for raw C-arrays
template <typename T, std::size_t N, typename Compare = std::less<>>
void heapSort(T (&arr)[N], Compare comp = Compare()) {
    heapSort(arr, arr + N, comp);
}

// 2. Stooge sort for iterator ranges
template <typename RandomIt, typename Compare = std::less<>>
void stoogeSort(RandomIt first, RandomIt last, Compare comp = Compare()) {
    std::ptrdiff_t count = std::distance(first, last);
    if (count <= 1) return;
    detail::stoogeSortImpl(first, 0, count - 1, comp);
}

// Stooge sort overload for standard containers
template <typename Container, typename Compare = std::less<>>
void stoogeSort(Container& container, Compare comp = Compare()) {
    stoogeSort(std::begin(container), std::end(container), comp);
}

// Stooge sort overload for raw C-arrays
template <typename T, std::size_t N, typename Compare = std::less<>>
void stoogeSort(T (&arr)[N], Compare comp = Compare()) {
    stoogeSort(arr, arr + N, comp);
}

} // namespace lab4