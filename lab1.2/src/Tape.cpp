#include "Tape.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>

Tape::Tape(Alphabet alphabet) : alphabet_(std::move(alphabet)) {}

char Tape::read(int64_t position) const noexcept {
    auto it = cells_.find(position);
    if (it != cells_.end()) {
        return it->second;
    }
    return alphabet_.getBlank();
}

void Tape::write(int64_t position, char symbol) {
    if (!alphabet_.isValid(symbol)) {
        throw std::invalid_argument(std::string("Tape::write: Symbol '") + symbol + 
                                    "' is not valid in the tape alphabet");
    }

    if (symbol == alphabet_.getBlank()) {
        cells_.erase(position);
    } else {
        cells_.insert_or_assign(position, symbol);
    }
}

void Tape::loadString(std::string_view input_word, int64_t start_position) {
    // Валидация всей цепочки перед изменением состояния ленты
    for (char c : input_word) {
        if (!alphabet_.isValid(c)) {
            throw std::invalid_argument(std::string("Tape::loadString: Invalid symbol '") + c +
                                        "' in input sequence");
        }
    }

    clear();
    int64_t current_pos = start_position;
    for (char c : input_word) {
        if (c != alphabet_.getBlank()) {
            cells_.insert_or_assign(current_pos, c);
        }
        ++current_pos;
    }
}

void Tape::clear() noexcept {
    cells_.clear();
}

bool Tape::empty() const noexcept {
    return cells_.empty();
}

std::pair<int64_t, int64_t> Tape::getBounds() const noexcept {
    if (cells_.empty()) {
        return {0, -1};
    }
    return {cells_.begin()->first, cells_.rbegin()->first};
}

std::string Tape::toString() const {
    if (cells_.empty()) {
        return "";
    }

    auto [min_pos, max_pos] = getBounds();
    std::string result;
    result.reserve(static_cast<std::size_t>(max_pos - min_pos + 1));

    for (int64_t pos = min_pos; pos <= max_pos; ++pos) {
        result.push_back(read(pos));
    }

    return result;
}

const Alphabet& Tape::getAlphabet() const noexcept {
    return alphabet_;
}