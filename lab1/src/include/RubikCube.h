/**
 * @file RubikCube.h
 * @author Твое Имя
 * @brief Объявление класса RubikCube и вспомогательных структур.
 * @details Содержит перечисления цветов, граней, а также интерфейс
 *          для вращения, валидации, перемешивания и сериализации кубика Рубика 3x3.
 */

#pragma once

#include <iostream>
#include <string>
#include <array>

/**
 * @brief Цвета наклеек кубика Рубика.
 */

enum class Color{
    White,
    Yellow,
    Green,
    Blue,
    Red,
    Orange
};

/**
 * @brief Грани кубика Рубика (нотация Singmaster).
 */

enum class Face {
    Up,
    Down,
    Front,
    Back,
    Left,
    Right
};

/**
 * @brief Класс, реализующий модель кубика Рубика 3x3.
 * @details Обеспечивает контроль инварианта собранности, поворот граней,
 *          генерацию случайных перестановок и потоковый ввод-вывод.
 */

class RubikCube {
private:
    /**
     * @brief Внутреннее состояние: 6 граней, на каждой по 9 клеток (3x3).
     * @details Порядок граней строго фиксирован индексами enum Face.
     */
    std::array<std::array<Color, 9>, 6> state_;

    /**
     * @brief Вспомогательная функция для безопасного преобразования enum Face в индекс массива.
     */
    static std::size_t toIndex(Face face) {
        return static_cast<std::size_t>(face);
    }

    /**
     * @brief Вспомогательный метод для поворота самой плоскости грани 3x3.
     * @param[in] face Грань, матрицу которой нужно повернуть.
     * @param[in] clockwise Направление: true — по часовой, false — против.
     */
    void rotateFaceMatrix(Face face, bool clockwise);

    public:
    /**
     * @brief Конструктор по умолчанию.
     * @details Создает идеально собранный кубик (каждая грань монохромна).
     */
    RubikCube();

    RubikCube(const RubikCube& other) = default;
    RubikCube& operator=(const RubikCube& other) = default;
    RubikCube(RubikCube&& other) = default;
    RubikCube& operator=(RubikCube&& other) = default;
    ~RubikCube() = default;

    /**
     * @brief Случайное перемешивание кубика.
     * @details Генерирует последовательность из валидных поворотов,
     *          сохраняя математическую разрешимость состояния.
     */
    void randomize();

    /**
     * @brief Загрузка расположения цветов из внешнего текстового файла.
     * @param[in] filePath Путь к файлу конфигурации.
     * @return true, если файл успешно прочитан и инвариант цветов соблюден (по 9 каждого цвета).
     * @return false, если файл поврежден, не существует или содержит ошибки.
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief Поворот заданной грани на 90 градусов.
     * @param[in] face Грань для поворота (Up, Down, Front, Back, Left, Right).
     * @param[in] clockwise Направление: true — по часовой стрелке, false — против часовой.
     */
    void rotateFace(Face face, bool clockwise = true);

    /**
     * @brief Проверка правильной расстановки цветных клеток (состояние победы).
     * @return true, если все 6 граней являются одноцветными, иначе false.
     */
    bool isSolved() const;

    /**
     * @brief Операторы сравнения двух кубиков на полное совпадение расположения цветов.
     */
    bool operator==(const RubikCube& rhs) const {
        return state_ == rhs.state_;
    }

    bool operator!=(const RubikCube& rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Потоковый вывод текстовой развертки кубика.
     */
    friend std::ostream& operator<<(std::ostream& os, const RubikCube& cube);

    /**
     * @brief Потоковый ввод состояния кубика.
     */
    friend std::istream& operator>>(std::istream& is, RubikCube& cube);
};