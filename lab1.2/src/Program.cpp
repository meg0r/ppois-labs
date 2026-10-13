#include "Program.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

Program::Program(State init_state, State halt_state)
    : initial_state_(std::move(init_state))
    , halt_state_(std::move(halt_state)) {}

void Program::addRule(const State& state, char symbol,
                      const State& next_state, char write_symbol, Direction dir) {
    rules_.insert_or_assign(RuleKey{state, symbol},
                            TransitionAction{next_state, write_symbol, dir});
}

bool Program::removeRule(const State& state, char symbol) {
    return rules_.erase(RuleKey{state, symbol}) > 0;
}

std::optional<TransitionAction> Program::getAction(const State& state, char symbol) const {
    auto it = rules_.find(RuleKey{state, symbol});
    if (it != rules_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::size_t Program::loadFromStream(std::istream& is) {
    std::string line;
    std::size_t loaded_count = 0;

    while (std::getline(is, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }

        std::istringstream iss(line);
        State cur_state;
        char read_sym{};
        State next_state;
        char write_sym{};
        char dir_char{};

        if (!(iss >> cur_state >> read_sym >> next_state >> write_sym >> dir_char)) {
            throw std::runtime_error("Program::loadFromStream: Invalid format in line: " + line);
        }

        Direction dir{};
        switch (dir_char) {
            case 'L':
            case 'l':
                dir = Direction::Left;
                break;
            case 'R':
            case 'r':
                dir = Direction::Right;
                break;
            case 'S':
            case 's':
            case 'N':
            case 'n':
                dir = Direction::Stay;
                break;
            default:
                throw std::runtime_error(std::string("Program::loadFromStream: Unknown direction '") + dir_char + "'");
        }

        addRule(cur_state, read_sym, next_state, write_sym, dir);
        ++loaded_count;
    }

    return loaded_count;
}

bool Program::isHaltState(const State& state) const noexcept {
    return state == halt_state_;
}

const State& Program::getInitialState() const noexcept {
    return initial_state_;
}

const State& Program::getHaltState() const noexcept {
    return halt_state_;
}

std::size_t Program::size() const noexcept {
    return rules_.size();
}

bool Program::empty() const noexcept {
    return rules_.empty();
}

void Program::clear() noexcept {
    rules_.clear();
}

const Program::RuleTable& Program::getAllRules() const noexcept {
    return rules_;
}