#pragma once

#include "Head.hpp"
#include "Program.hpp"
#include "Tape.hpp"
#include "Transition.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

/**
 * @file TuringMachine.hpp
 * @brief Фасад симулятора детерминированной машины Тьюринга.
 *
 * Машина функционирует как вычислитель.
 * Терминальный останов происходит исключительно в двух случаях:
 *   1. Достигнуто терминальное halt-состояние (HaltedByState).
 *   2. В таблице отсутствует правило для пары (state, symbol) (HaltedNoRule).
 */

/**
 * @brief Статус выполнения машины Тьюринга.
 */
enum class ExecutionStatus {
    Running,          ///< Машина активна и готова к следующему шагу
    HaltedByState,    ///< Терминальный останов: достигнуто halt-состояние
    HaltedNoRule,     ///< Терминальный останов: нет перехода для (state, symbol)
    MaxStepsExceeded  ///< Исчерпана квота шагов вызова run() (статус машины: Running)
};

class TuringMachine {
private:
    Tape tape_;
    Head head_;
    Program program_;
    State current_state_;
    ExecutionStatus status_ = ExecutionStatus::Running;
    std::size_t step_counter_ = 0;

    void moveHead(Direction dir) noexcept;
    void initializeRunState();

public:
    /**
     * @brief Конструктор машины Тьюринга.
     * @param alphabet Алфавит (передается во владение ленте Tape).
     * @param program Таблица правил переходов.
     * @param start_pos Начальная позиция каретки (по умолчанию 0).
     */
    TuringMachine(Alphabet alphabet, Program program, int64_t start_pos = 0);

    /**
     * @brief Сбрасывает машину в исходное состояние и загружает слово на ленту.
     * 
     * Очищает ленту, устанавливает позицию каретки, переводит состояние в initial_state,
     * обнуляет счетчик шагов и переводит статус в Running (или HaltedByState, если initial == halt).
     * 
     * @param input Входное слово.
     * @param start_pos Начальная координата первого символа и каретки.
     */
    void reset(std::string_view input = "", int64_t start_pos = 0);

    /**
     * @brief Выполняет ровно один такт работы машины.
     * 
     * Если такт успешно выполнен и привел к halt-состоянию, счетчик инкрементируется,
     * статус переходит в HaltedByState, метод возвращает false.
     * Если правило перехода отсутствует, никакого действия не совершается:
     * счетчик НЕ инкрементируется, статус переходит в HaltedNoRule, метод возвращает false.
     * 
     * @return true, если шаг выполнен и машина все еще Running; иначе false.
     */
    bool step();

    /**
     * @brief Выполняет цикл шагов в пределах заданной квоты max_steps.
     * 
     * Если лимит исчерпан, возвращает ExecutionStatus::MaxStepsExceeded. При этом
     * внутренний статус машины остается Running (выполнение можно продолжить следующим вызовом).
     * 
     * @param max_steps Квота шагов на текущий вызов. Значение 0 не выполняет шагов.
     * @return Итоговый статус (HaltedByState, HaltedNoRule, Running или MaxStepsExceeded).
     */
    ExecutionStatus run(std::size_t max_steps = 100'000);

    [[nodiscard]] bool isHalted() const noexcept;
    [[nodiscard]] ExecutionStatus getStatus() const noexcept;
    [[nodiscard]] const State& getCurrentState() const noexcept;
    [[nodiscard]] std::size_t getStepCount() const noexcept;

    [[nodiscard]] const Tape& getTape() const noexcept;
    [[nodiscard]] const Head& getHead() const noexcept;
    [[nodiscard]] const Program& getProgram() const noexcept;
    [[nodiscard]] const Alphabet& getAlphabet() const noexcept;
};