#include "TuringMachine.hpp"

#include <cstdint>
#include <utility>

TuringMachine::TuringMachine(Alphabet alphabet, Program program, int64_t start_pos)
    : tape_(std::move(alphabet))
    , head_(start_pos)
    , program_(std::move(program))
    , current_state_(program_.getInitialState())
    , status_(ExecutionStatus::Running)
    , step_counter_(0) {
    initializeRunState();
}

void TuringMachine::initializeRunState() {
    current_state_ = program_.getInitialState();
    step_counter_ = 0;
    if (program_.isHaltState(current_state_)) {
        status_ = ExecutionStatus::HaltedByState;
    } else {
        status_ = ExecutionStatus::Running;
    }
}

void TuringMachine::moveHead(Direction dir) noexcept {
    switch (dir) {
        case Direction::Left:
            --head_;
            break;
        case Direction::Right:
            ++head_;
            break;
        case Direction::Stay:
            break;
    }
}

void TuringMachine::reset(std::string_view input, int64_t start_pos) {
    tape_.loadString(input, start_pos);
    head_.setPosition(start_pos);
    initializeRunState();
}

bool TuringMachine::step() {
    if (status_ != ExecutionStatus::Running) {
        return false;
    }

    char current_sym = tape_.read(head_.getPosition());
    auto action = program_.getAction(current_state_, current_sym);

    // Действие не найдено: шаг не совершён, счётчик не инкрементируется
    if (!action.has_value()) {
        status_ = ExecutionStatus::HaltedNoRule;
        return false;
    }

    // Применение перехода
    tape_.write(head_.getPosition(), action->write_symbol);
    moveHead(action->move_direction);
    current_state_ = action->next_state;
    ++step_counter_;

    // Проверка терминального состояния
    if (program_.isHaltState(current_state_)) {
        status_ = ExecutionStatus::HaltedByState;
        return false;
    }

    return true;
}

ExecutionStatus TuringMachine::run(std::size_t max_steps) {
    if (status_ != ExecutionStatus::Running) {
        return status_;
    }

    for (std::size_t i = 0; i < max_steps; ++i) {
        if (!step()) {
            return status_; // Завершился штатно по HaltedByState или HaltedNoRule
        }
    }

    // Квота исчерпана: машина остаётся в статусе Running, возвращаем MaxStepsExceeded
    return ExecutionStatus::MaxStepsExceeded;
}

bool TuringMachine::isHalted() const noexcept {
    return status_ == ExecutionStatus::HaltedByState || status_ == ExecutionStatus::HaltedNoRule;
}

ExecutionStatus TuringMachine::getStatus() const noexcept {
    return status_;
}

const State& TuringMachine::getCurrentState() const noexcept {
    return current_state_;
}

std::size_t TuringMachine::getStepCount() const noexcept {
    return step_counter_;
}

const Tape& TuringMachine::getTape() const noexcept {
    return tape_;
}

const Head& TuringMachine::getHead() const noexcept {
    return head_;
}

const Program& TuringMachine::getProgram() const noexcept {
    return program_;
}

const Alphabet& TuringMachine::getAlphabet() const noexcept {
    return tape_.getAlphabet();
}