#include "Alphabet.hpp"

Alphabet::Alphabet(char blank) : blank_symbol_(blank) {
    symbols_.insert(blank);
}

void Alphabet::addSymbol(char c) {
    symbols_.insert(c);
}

void Alphabet::addSymbols(std::string_view chars) {
    for (char c : chars) {
        symbols_.insert(c);
    }
}

bool Alphabet::isValid(char c) const noexcept {
    return symbols_.find(c) != symbols_.end();
}

char Alphabet::getBlank() const noexcept {
    return blank_symbol_;
}