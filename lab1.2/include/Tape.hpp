#pragma once

#include "Alphabet.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <utility>

/**
 * @file Tape.hpp
 * @brief Двусторонняя бесконечная разреженная лента машины Тьюринга.
 *
 * Координаты ячеек задаются знаковым целым int64_t.
 * Неинициализированные ячейки неявно содержат Alphabet::getBlank().
 *
 * Инвариант: во внутренней таблице cells_ хранятся ТОЛЬКО символы,
 * отличные от blank. Запись blank-символа приводит к удалению ячейки.
 */
class Tape {
private:
    Alphabet alphabet_;              ///< Алфавит для валидации данных и получения blank
    std::map<int64_t, char> cells_;  ///< Разреженная таблица ячеек (только не-blank)

public:
    /**
     * @brief Конструктор ленты.
     * @param alphabet Алфавит ленты (по умолчанию пустой символ '_').
     */
    explicit Tape(Alphabet alphabet = Alphabet{'_'});

    /**
     * @brief Считывает символ в заданной позиции ленты (сложность O(log N)).
     * @param position Знаковая координата ячейки.
     * @return Символ ячейки или alphabet_.getBlank(), если ячейка пуста.
     */
    [[nodiscard]] char read(int64_t position) const noexcept;

    /**
     * @brief Записывает символ в указанную позицию.
     *
     * Если symbol == alphabet_.getBlank(), ячейка удаляется из памяти,
     * сохраняя инвариант разреженности ленты.
     *
     * @param position Знаковая координата ячейки.
     * @param symbol Записываемый символ.
     * @throws std::invalid_argument Если символ отсутствует в alphabet_.
     */
    void write(int64_t position, char symbol);

    /**
     * @brief Загружает входное слово на ленту, предварительно полностью очищая её.
     *
     * Поддерживаются произвольные знаковые start_position (включая отрицательные).
     * Если входное слово содержит blank-символы, соответствующие позиции остаются
     * незанятыми («дырками»).
     *
     * @param input_word Строка для загрузки.
     * @param start_position Координата начального символа (по умолчанию 0).
     * @throws std::invalid_argument Если во входной строке встретился символ вне алфавита.
     */
    void loadString(std::string_view input_word, int64_t start_position = 0);

    /**
     * @brief Полностью очищает ленту (все ячейки становятся blank).
     */
    void clear() noexcept;

    /**
     * @brief Проверяет, пуста ли лента (содержит ли только blank-ячейки).
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Возвращает границы значащей части ленты [min_pos, max_pos].
     *
     * Инвариант: min_pos <= max_pos. Значение {0, -1} является маркером пустой ленты.
     * @return Пара [min_pos, max_pos] или {0, -1}, если лента пуста.
     */
    [[nodiscard]] std::pair<int64_t, int64_t> getBounds() const noexcept;

    /**
     * @brief Формирует срез значащей части ленты от min_pos до max_pos.
     *
     * Любые незаписанные «дырки» между крайними символами подменяются на blank-символ.
     * Если лента пуста, возвращается пустая строка.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Возвращает константную ссылку на алфавит ленты.
     */
    [[nodiscard]] const Alphabet& getAlphabet() const noexcept;
};