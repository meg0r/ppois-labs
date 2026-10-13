#pragma once

#include "Transition.hpp"

#include <cstddef>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <utility>

/**
 * @file Program.hpp
 * @brief Таблица правил переходов машины Тьюринга.
 * 
 * Инкапсулирует отображение (state, symbol) -> TransitionAction.
 * Машина останавливается либо при переходе в halt-состояние,
 * либо если для текущей пары (state, symbol) правило отсутствует (getAction вернёт std::nullopt).
 */
class Program {
public:
    /// Ключ таблицы: (текущее состояние, обозреваемый символ)
    using RuleKey = std::pair<State, char>;
    /// Таблица правил переходов
    using RuleTable = std::map<RuleKey, TransitionAction>;

private:
    RuleTable rules_;       ///< Набор правил переходов
    State initial_state_;   ///< Стартовое состояние (по умолчанию "q0")
    State halt_state_;      ///< Терминальное состояние останова (по умолчанию "halt")

public:
    /**
     * @brief Конструктор программы.
     * @param init_state Имя начального состояния.
     * @param halt_state Имя терминального состояния.
     */
    explicit Program(State init_state = "q0", State halt_state = "halt");

    /**
     * @brief Добавляет правило в таблицу. Если правило для пары (state, symbol) уже есть — заменяет его.
     * @note Внутри используется std::map::insert_or_assign, так как дефолтный конструктор TransitionAction удален.
     */
    void addRule(const State& state, char symbol,
                  const State& next_state, char write_symbol, Direction dir);

    /**
     * @brief Удаляет правило для заданной пары (состояние, символ).
     * @return true, если правило существовало и было удалено, иначе false.
     */
    bool removeRule(const State& state, char symbol);

    /**
     * @brief Ищет действие для текущего состояния и считанного символа.
     * @return std::optional с действием или std::nullopt, если правило не найдено (останов).
     */
    [[nodiscard]] std::optional<TransitionAction> getAction(const State& state, char symbol) const;

    /**
     * @brief Загружает правила из текстового потока.
     * Формат строки: <cur_state> <read_sym> <next_state> <write_sym> <dir>
     * Пустые строки и комментарии '#' пропускаются. Пробельные символы ленты не поддерживаются.
     * @param is Входной поток (файл или stringstream).
     * @return Количество успешно загруженных правил.
     * @throws std::runtime_error При синтаксической ошибке или неизвестном направлении.
     */
    std::size_t loadFromStream(std::istream& is);

    /**
     * @brief Проверяет, является ли состояние терминальным (halt).
     */
    [[nodiscard]] bool isHaltState(const State& state) const noexcept;

    [[nodiscard]] const State& getInitialState() const noexcept;
    [[nodiscard]] const State& getHaltState() const noexcept;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    void clear() noexcept;

    [[nodiscard]] const RuleTable& getAllRules() const noexcept;
};