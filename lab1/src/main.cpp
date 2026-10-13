#include "include/RubikCube.h"
#include <iostream>
#include <string>
#include <sstream>

void printHelp() {
    std::cout << "\n--- AVAILABLE COMMANDS ---\n"
              << "  U, D, F, B, L, R       - Rotate face clockwise\n"
              << "  U', D', F', B', L', R' - Rotate face counter-clockwise\n"
              << "  rand                   - Randomize cube state\n"
              << "  load <filename>        - Load configuration from file\n"
              << "  check                  - Check if the cube is solved\n"
              << "  help                   - Display this help message\n"
              << "  exit                   - Exit application\n"
              << "--------------------------\n";
}

int main() {
    RubikCube cube;

    std::cout << "========================================\n"
              << "        RUBIK'S CUBE 3x3 SIMULATOR      \n"
              << "========================================\n";
    printHelp();

    std::cout << "\nInitial cube state:\n" << cube << "\n";

    std::string line;
    while (true) {
        std::cout << "\nEnter command > ";
        if (!std::getline(std::cin, line)) {
            break; // Обработка EOF
        }

        std::stringstream ss(line);
        std::string command;
        if (!(ss >> command)) {
            continue; // Пустой ввод (просто Enter)
        }

        if (command == "exit") {
            std::cout << "Exiting simulation. Goodbye!\n";
            break;
        }

        if (command == "help") {
            printHelp();
            continue;
        }

        if (command == "rand") {
            cube.randomize();
            std::cout << "\nCube randomized!\n" << cube << "\n";
            continue;
        }

        if (command == "check") {
            if (cube.isSolved()) {
                std::cout << "CONGRATULATIONS! The cube is fully solved!\n";
            } else {
                std::cout << "The cube is NOT solved yet.\n";
            }
            continue;
        }

        if (command == "load") {
            std::string filename;
            if (!(ss >> filename)) {
                std::cout << "Error: Missing filename argument! Usage: load <filename>\n";
            } else if (cube.loadFromFile(filename)) {
                std::cout << "\nFile \"" << filename << "\" loaded successfully!\n" << cube << "\n";
            } else {
                std::cout << "Error: Failed to load file or color balance is invalid!\n";
            }
            continue;
        }

        // Повороты граней
        bool clockwise = true;
        std::string faceStr = command;

        if (faceStr.length() == 2 && faceStr.back() == '\'') {
            clockwise = false;
            faceStr.pop_back();
        }

        if (faceStr == "U") {
            cube.rotateFace(Face::Up, clockwise);
        } else if (faceStr == "D") {
            cube.rotateFace(Face::Down, clockwise);
        } else if (faceStr == "F") {
            cube.rotateFace(Face::Front, clockwise);
        } else if (faceStr == "B") {
            cube.rotateFace(Face::Back, clockwise);
        } else if (faceStr == "L") {
            cube.rotateFace(Face::Left, clockwise);
        } else if (faceStr == "R") {
            cube.rotateFace(Face::Right, clockwise);
        } else {
            std::cout << "Unknown command! Type 'help' to see valid commands.\n";
            continue;
        }

        // Печать состояния после поворота
        std::cout << "\nState after move:\n" << cube << "\n";

        // Проверка условия победы
        if (cube.isSolved()) {
            std::cout << "*** VICTORY! The cube is solved! ***\n";
        }
    }

    return 0;
}