#pragma once

#include <cstdint>

/**
 * @file Head.hpp
 * @brief Класс каретки (головки) машины Тьюринга.
 * 
 * Инкапсулирует позицию на ленте и поддерживает перемещение
 * через операторы инкремента (вправо) и декремента (влево).
 */
class Head {
private:
    int64_t position_; ///< Текущий индекс ячейки на ленте

public:
    /**
     * @brief Конструктор каретки.
     * @param start_pos Начальная позиция на ленте (по умолчанию 0).
     */
    explicit Head(int64_t start_pos = 0) noexcept;

    /**
     * @brief Получить текущую позицию каретки.
     * @return Индекс текущей ячейки.
     */
    [[nodiscard]] int64_t getPosition() const noexcept;

    /**
     * @brief Установить произвольную позицию каретки.
     * @param pos Новый индекс ячейки.
     */
    void setPosition(int64_t pos) noexcept;

    /**
     * @brief Префиксный инкремент (++head). Сдвиг вправо.
     * @return Ссылка на текущий объект после сдвига.
     */
    Head& operator++() noexcept;

    /**
     * @brief Постфиксный инкремент (head++). Сдвиг вправо.
     * @return Копия объекта до сдвига.
     */
    Head operator++(int) noexcept;

    /**
     * @brief Префиксный декремент (--head). Сдвиг влево.
     * @return Ссылка на текущий объект после сдвига.
     */
    Head& operator--() noexcept;

    /**
     * @brief Постфиксный декремент (head--). Сдвиг влево.
     * @return Копия объекта до сдвига.
     */
    Head operator--(int) noexcept;

    /**
     * @brief Сравнение позиций двух кареток на равенство.
     */
    bool operator==(const Head& other) const noexcept;

    /**
     * @brief Сравнение позиций двух кареток на неравенство.
     */
    bool operator!=(const Head& other) const noexcept;
};