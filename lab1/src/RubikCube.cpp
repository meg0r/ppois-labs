#include "include/RubikCube.h"
#include <fstream>
#include <random>

// ==========================================
// Вспомогательные функции для символов цвета
// ==========================================

static char colorToChar(Color c) {
    switch (c) {
        case Color::White:  return 'W';
        case Color::Yellow: return 'Y';
        case Color::Green:  return 'G';
        case Color::Blue:   return 'B';
        case Color::Red:    return 'R';
        case Color::Orange: return 'O';
    }
    return '?';
}

static bool charToColor(char ch, Color& outColor) {
    switch (ch) {
        case 'W': case 'w': outColor = Color::White;  return true;
        case 'Y': case 'y': outColor = Color::Yellow; return true;
        case 'G': case 'g': outColor = Color::Green;  return true;
        case 'B': case 'b': outColor = Color::Blue;   return true;
        case 'R': case 'r': outColor = Color::Red;    return true;
        case 'O': case 'o': outColor = Color::Orange; return true;
        default:  return false;
    }
}

// ==========================================
// Конструктор: собранный кубик
// ==========================================

RubikCube::RubikCube() {
    // Начальные цвета: Up=White, Down=Yellow, Front=Green,
    // Back=Blue, Left=Orange, Right=Red
    Color initialColors[6] = {
        Color::White, Color::Yellow, Color::Green,
        Color::Blue,  Color::Orange, Color::Red
    };

    for (std::size_t f = 0; f < 6; ++f) {
        for (std::size_t i = 0; i < 9; ++i) {
            state_[f][i] = initialColors[f];
        }
    }
}

// ==========================================
// Поворот матрицы 3х3 самой грани (без соседей)
// ==========================================

void RubikCube::rotateFaceMatrix(Face face, bool clockwise) {
    std::size_t f = toIndex(face);
    auto oldFace = state_[f];

    if (clockwise) {
        // Поворот по часовой стрелке
        state_[f][0] = oldFace[6]; state_[f][1] = oldFace[3]; state_[f][2] = oldFace[0];
        state_[f][3] = oldFace[7]; state_[f][4] = oldFace[4]; state_[f][5] = oldFace[1];
        state_[f][6] = oldFace[8]; state_[f][7] = oldFace[5]; state_[f][8] = oldFace[2];
    } else {
        // Поворот против часовой стрелки
        state_[f][0] = oldFace[2]; state_[f][1] = oldFace[5]; state_[f][2] = oldFace[8];
        state_[f][3] = oldFace[1]; state_[f][4] = oldFace[4]; state_[f][5] = oldFace[7];
        state_[f][6] = oldFace[0]; state_[f][7] = oldFace[3]; state_[f][8] = oldFace[6];
    }
}

// ==========================================
// Поворот грани и смежных рёбер
// ==========================================

void RubikCube::rotateFace(Face face, bool clockwise) {
    // 1 поворот против часовой стрелки равен 3 поворотам по часовой стрелке
    if (!clockwise) {
        rotateFace(face, true);
        rotateFace(face, true);
        rotateFace(face, true);
        return;
    }

    // 1. Поворачиваем саму матрицу грани 3x3 (всегда только по часовой)
    rotateFaceMatrix(face, true);

    // Индексы граней
    std::size_t u = toIndex(Face::Up);
    std::size_t d = toIndex(Face::Down);
    std::size_t f = toIndex(Face::Front);
    std::size_t b = toIndex(Face::Back);
    std::size_t l = toIndex(Face::Left);
    std::size_t r = toIndex(Face::Right);

    // 2. Смещение смежных полосок по 3 клетки по часовой стрелке
    if (face == Face::Up) {
        Color temp[3] = {state_[b][0], state_[b][1], state_[b][2]};
        for (int i = 0; i < 3; ++i) {
            state_[b][i] = state_[l][i];
            state_[l][i] = state_[f][i];
            state_[f][i] = state_[r][i];
            state_[r][i] = temp[i];
        }
    } else if (face == Face::Down) {
        Color temp[3] = {state_[f][6], state_[f][7], state_[f][8]};
        for (int i = 0; i < 3; ++i) {
            state_[f][6 + i] = state_[l][6 + i];
            state_[l][6 + i] = state_[b][6 + i];
            state_[b][6 + i] = state_[r][6 + i];
            state_[r][6 + i] = temp[i];
        }
    } else if (face == Face::Front) {
        Color temp[3] = {state_[u][6], state_[u][7], state_[u][8]};
        state_[u][6] = state_[l][8]; state_[u][7] = state_[l][5]; state_[u][8] = state_[l][2];
        state_[l][2] = state_[d][0]; state_[l][5] = state_[d][1]; state_[l][8] = state_[d][2];
        state_[d][0] = state_[r][6]; state_[d][1] = state_[r][3]; state_[d][2] = state_[r][0];
        state_[r][0] = temp[0];      state_[r][3] = temp[1];      state_[r][6] = temp[2];
    } else if (face == Face::Back) {
        Color temp[3] = {state_[u][2], state_[u][1], state_[u][0]};
        state_[u][0] = state_[r][2]; state_[u][1] = state_[r][5]; state_[u][2] = state_[r][8];
        state_[r][2] = state_[d][8]; state_[r][5] = state_[d][7]; state_[r][8] = state_[d][6];
        state_[d][6] = state_[l][0]; state_[d][7] = state_[l][3]; state_[d][8] = state_[l][6];
        state_[l][0] = temp[0];      state_[l][3] = temp[1];      state_[l][6] = temp[2];
    } else if (face == Face::Left) {
        Color temp[3] = {state_[u][0], state_[u][3], state_[u][6]};
        state_[u][0] = state_[b][8]; state_[u][3] = state_[b][5]; state_[u][6] = state_[b][2];
        state_[b][2] = state_[d][6]; state_[b][5] = state_[d][3]; state_[b][8] = state_[d][0];
        state_[d][0] = state_[f][0]; state_[d][3] = state_[f][3]; state_[d][6] = state_[f][6];
        state_[f][0] = temp[0];      state_[f][3] = temp[1];      state_[f][6] = temp[2];
    } else if (face == Face::Right) {
        Color temp[3] = {state_[u][8], state_[u][5], state_[u][2]};
        state_[u][2] = state_[f][2]; state_[u][5] = state_[f][5]; state_[u][8] = state_[f][8];
        state_[f][2] = state_[d][2]; state_[f][5] = state_[d][5]; state_[f][8] = state_[d][8];
        state_[d][2] = state_[b][6]; state_[d][5] = state_[b][3]; state_[d][8] = state_[b][0];
        state_[b][0] = temp[2];      state_[b][3] = temp[1];      state_[b][6] = temp[0];
    }
}

// ==========================================
// Случайное перемешивание
// ==========================================

void RubikCube::randomize() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> faceDist(0, 5);
    std::uniform_int_distribution<int> dirDist(0, 1);

    // 25 случайных ходов гарантируют сохранение сборки кубика
    for (int i = 0; i < 25; ++i) {
        Face randomFace = static_cast<Face>(faceDist(gen));
        bool clockwise = (dirDist(gen) == 1);
        rotateFace(randomFace, clockwise);
    }
}

// ==========================================
// Проверка собранности
// ==========================================

bool RubikCube::isSolved() const {
    for (std::size_t f = 0; f < 6; ++f) {
        Color faceColor = state_[f][0];
        for (std::size_t i = 1; i < 9; ++i) {
            if (state_[f][i] != faceColor) {
                return false;
            }
        }
    }
    return true;
}

// ==========================================
// Загрузка из файла
// ==========================================

bool RubikCube::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::array<std::array<Color, 9>, 6> tempState;
    int colorCounts[6] = {0}; // Счётчик: каждого цвета должно быть ровно 9
    char ch;

    for (std::size_t f = 0; f < 6; ++f) {
        for (std::size_t i = 0; i < 9; ++i) {
            if (!(file >> ch)) {
                return false; // Файл оборван
            }
            Color c;
            if (!charToColor(ch, c)) {
                return false; // Неизвестная буква
            }
            tempState[f][i] = c;
            colorCounts[static_cast<std::size_t>(c)]++;
        }
    }

    // Проверяем инвариант: ровно по 9 наклеек каждого цвета
    for (int count : colorCounts) {
        if (count != 9) {
            return false;
        }
    }

    state_ = tempState;
    return true;
}

// ==========================================
// Потоковый вывод (развёртка)
// ==========================================

std::ostream& operator<<(std::ostream& os, const RubikCube& cube) {
    const char* faceNames[6] = {"UP", "DOWN", "FRONT", "BACK", "LEFT", "RIGHT"};
    for (std::size_t f = 0; f < 6; ++f) {
        os << "[" << faceNames[f] << "]\n";
        for (std::size_t r = 0; r < 3; ++r) {
            for (std::size_t c = 0; c < 3; ++c) {
                os << colorToChar(cube.state_[f][r * 3 + c]) << " ";
            }
            os << "\n";
        }
    }
    return os;
}

// ==========================================
// Потоковый ввод
// ==========================================

std::istream& operator>>(std::istream& is, RubikCube& cube) {
    std::array<std::array<Color, 9>, 6> tempState;
    char ch;
    for (std::size_t f = 0; f < 6; ++f) {
        for (std::size_t i = 0; i < 9; ++i) {
            if (is >> ch) {
                Color c;
                if (charToColor(ch, c)) {
                    tempState[f][i] = c;
                }
            }
        }
    }
    cube.state_ = tempState;
    return is;
}