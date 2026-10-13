#pragma once

#include <string>
#include <utility>

/**
 * @file Transition.hpp
 * @brief Базовые типы переходов машины Тьюринга.
 *
 * Используется совместно Program и TuringMachine, чтобы не дублировать
 * Direction и описание действия.
 */

/// Строковый идентификатор состояния ("q0", "halt", "scan")
using State = std::string;

/**
 * @brief Направление перемещения каретки.
 */
enum class Direction {
    Left,   ///< Сдвиг на одну ячейку влево
    Right,  ///< Сдвиг на одну ячейку вправо
    Stay    ///< Головка остаётся на месте (расширение классической МТ)
};

/**
 * @brief Действие, выполняемое при срабатывании правила.
 *
 * Создаётся только целиком — дефолтный конструктор удалён,
 * чтобы исключить появление неполных правил с пустыми полями.
 */
struct TransitionAction {
    State     next_state;
    char      write_symbol;
    Direction move_direction;

    TransitionAction() = delete;

    TransitionAction(State ns, char ws, Direction d)
        : next_state(std::move(ns))
        , write_symbol(ws)
        , move_direction(d) {}

    bool operator==(const TransitionAction& other) const noexcept {
        return next_state == other.next_state &&
               write_symbol == other.write_symbol &&
               move_direction == other.move_direction;
    }

    bool operator!=(const TransitionAction& other) const noexcept {
        return !(*this == other);
    }
};