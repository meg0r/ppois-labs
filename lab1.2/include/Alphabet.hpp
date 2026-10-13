#pragma once

#include <unordered_set>
#include <string_view>

/**
 * @file Alphabet.hpp
 * @brief Класс алфавита машины Тьюринга.
 */
class Alphabet {
private:
    std::unordered_set<char> symbols_;
    const char blank_symbol_;

public:
    explicit Alphabet(char blank = '_');

    void addSymbol(char c);
    void addSymbols(std::string_view chars);
    [[nodiscard]] bool isValid(char c) const noexcept;
    [[nodiscard]] char getBlank() const noexcept;
};