#if __has_include(<UnitTest++/UnitTest++.h>)
    #include <UnitTest++/UnitTest++.h>
#else
    #include "UnitTest++.h"
#endif

#include "RubikCube.h"
#include <fstream>
#include <cstdio>

// 1. Состояние по умолчанию: новый кубик должен быть собран
TEST(DefaultConstructor_ShouldBeSolved) {
    RubikCube cube;
    CHECK(cube.isSolved());
}

// 2. Единичный поворот грани нарушает собранность
TEST(RotateFace_SingleTurn_BreaksSolvedState) {
    RubikCube cube;
    cube.rotateFace(Face::Up, true);
    CHECK(!cube.isSolved());
}

// 3. Полный оборот: 4 поворота по часовой возвращают грань на место
TEST(RotateFace_FourTimesClockwise_RestoresOriginalState) {
    RubikCube cube;
    for (int i = 0; i < 4; ++i) {
        cube.rotateFace(Face::Front, true);
    }
    CHECK(cube.isSolved());
}

// 4. Взаимная компенсация противоположных вращений
TEST(RotateFace_OppositeRotations_CancelOut) {
    RubikCube cube;
    cube.rotateFace(Face::Right, true);
    cube.rotateFace(Face::Right, false);
    CHECK(cube.isSolved());
}

// 5. Операторы сравнения (== и !=)
TEST(Operators_EqualityAndInequality_WorkCorrectly) {
    RubikCube first;
    RubikCube second;

    CHECK(first == second);
    CHECK(!(first != second));

    second.rotateFace(Face::Left, true);
    CHECK(first != second);
    CHECK(!(first == second));
}

// 6. Глубокое копирование: копия не мутирует оригинал
TEST(CopyConstructor_ModificationDoesNotAffectOriginal) {
    RubikCube original;
    RubikCube copy(original);

    copy.rotateFace(Face::Down, true);
    CHECK(original.isSolved());
    CHECK(!copy.isSolved());
    CHECK(original != copy);
}

// 7. Оператор присваивания: независимость состояния
TEST(AssignmentOperator_StateIndependence) {
    RubikCube cube1;
    RubikCube cube2;

    cube2.rotateFace(Face::Back, true);
    cube1 = cube2;
    CHECK(cube1 == cube2);

    cube1.rotateFace(Face::Back, false);
    CHECK(cube1.isSolved());
    CHECK(!cube2.isSolved());
}

// 8. Устойчивый тест на перемешивание (защита от случайных совпадений)
TEST(Randomize_ModifiesCubeState) {
    RubikCube original;
    bool stateChanged = false;

    // Делаем до 3 попыток на случай крайне маловероятного математического совпадения
    for (int attempt = 0; attempt < 3; ++attempt) {
        RubikCube randomizedCube;
        randomizedCube.randomize();
        if (randomizedCube != original) {
            stateChanged = true;
            break;
        }
    }

    CHECK(stateChanged);
}

// 9. Корректный отказ при чтении несуществующего файла
TEST(LoadFromFile_NonExistentFile_ReturnsFalse) {
    RubikCube cube;
    // Уникальное имя с префиксом теста для изоляции
    bool success = cube.loadFromFile("rubik_test_missing_file_89234729384.txt");
    CHECK(!success);
}

// 10. Проверка валидного и повреждённого файлов
TEST(LoadFromFile_ValidAndIncompleteFiles) {
    RubikCube cube;

    // А: Ровно 54 слитных символа в порядке граней: Up, Down, Front, Back, Left, Right
    const char* validPath = "temp_valid_cube_54_raw.txt";
    std::ofstream validFile(validPath, std::ios::trunc);
    CHECK(validFile.is_open());

    const char colorSequence[6] = {'W', 'Y', 'G', 'B', 'O', 'R'};
    for (int face = 0; face < 6; ++face) {
        for (int sticker = 0; sticker < 9; ++sticker) {
            validFile << colorSequence[face];
        }
    }
    validFile.close();

    CHECK(cube.loadFromFile(validPath));
    CHECK(cube.isSolved());
    std::remove(validPath);

    // Б: Повреждённый файл (меньше 54 символов)
    const char* brokenPath = "temp_broken_cube_3_raw.txt";
    std::ofstream brokenFile(brokenPath, std::ios::trunc);
    CHECK(brokenFile.is_open());

    brokenFile << "WYG";
    brokenFile.close();

    CHECK(!cube.loadFromFile(brokenPath));
    std::remove(brokenPath);
}

// Точка входа для запуска набора тестов
int main() {
    return UnitTest::RunAllTests();
}