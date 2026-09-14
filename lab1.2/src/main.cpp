#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include "Alphabet.hpp"
#include "Program.hpp"
#include "TuringMachine.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
namespace {

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string readLineTrimmed() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void printHeader() {
    std::cout << "\n==================================================\n";
    std::cout << "           СИМУЛЯТОР МАШИНЫ ТЬЮРИНГА (CLI)        \n";
    std::cout << "==================================================\n";
}

void printState(const TuringMachine& tm) {
    std::cout << "\n--- Состояние автомата ---\n";
    std::cout << "Текущее состояние: " << tm.getCurrentState() << '\n';
    std::cout << "Позиция каретки:   " << tm.getHead().getPosition() << '\n';
    std::cout << "Количество шагов:  " << tm.getStepCount() << '\n';
    std::cout << "Содержимое ленты:  \"" << tm.getTape().toString() << "\"\n";
    
    int64_t head_pos = tm.getHead().getPosition();
    int64_t left = head_pos;
    int64_t right = head_pos;

    if (!tm.getTape().empty()) {
        auto bounds = tm.getTape().getBounds();
        left = std::min(bounds.first, head_pos);
        right = std::max(bounds.second, head_pos);
    }

    left -= 1;
    right += 1;

    std::cout << "Координаты: ";
    for (int64_t i = left; i <= right; ++i) {
        std::cout << std::setw(4) << i;
    }
    std::cout << "\nЛента:      ";
    for (int64_t i = left; i <= right; ++i) {
        std::cout << std::setw(4) << tm.getTape().read(i);
    }
    std::cout << "\nКаретка:    ";
    for (int64_t i = left; i <= right; ++i) {
        if (i == head_pos) {
            std::cout << "   ^";
        } else {
            std::cout << "    ";
        }
    }
    std::cout << "\n";
}

void printMenu() {
    std::cout << "\n[МЕНЮ УПРАВЛЕНИЯ]\n"
              << "1. Задать начальное слово на ленте\n"
              << "2. Выполнить один такт (Step)\n"
              << "3. Запустить непрерывное выполнение (Run)\n"
              << "4. Добавить правило перехода вручную\n"
              << "5. Показать таблицу правил программы\n"
              << "6. Добавить символ в рабочий алфавит\n"
              << "7. Загрузить пример: Унарный инкремент (+1 к '111')\n"
              << "0. Выход\n"
              << "Выберите действие > ";
}

} // namespace

int main() {
    SetConsoleOutputCP(65001);

    Alphabet alphabet{'_'};
    alphabet.addSymbols("01");

    Program program("q0", "halt");
    auto tm = std::make_unique<TuringMachine>(alphabet, program);

    printHeader();

    while (true) {
        printMenu();
        int choice = -1;
        
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                break;
            }
            std::cout << "\n[Ошибка]: Введите корректный номер пункта меню!\n";
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (choice == 0) {
            std::cout << "\nРабота симулятора завершена.\n";
            break;
        }

        switch (choice) {
            case 1: {
                std::cout << "Введите начальное слово (для пустой ленты введите _): ";
                std::string input = readLineTrimmed();
                if (input == "_") {
                    input = "";
                }

                int64_t start_pos = 0;
                std::cout << "Введите начальную позицию каретки [целое число, Enter = 0]: ";
                std::string pos_str = readLineTrimmed();
                if (!pos_str.empty()) {
                    std::istringstream iss(pos_str);
                    if (!(iss >> start_pos)) {
                        std::cout << "[Внимание]: Некорректный ввод позиции. Установлено 0.\n";
                        start_pos = 0;
                    }
                }

                try {
                    tm->reset(input, start_pos);
                    std::cout << "[Успех]: Лента успешно инициализирована.\n";
                    printState(*tm);
                } catch (const std::exception& e) {
                    std::cout << "[Ошибка валидации]: " << e.what() << '\n';
                }
                break;
            }

            case 2: {
                if (tm->isHalted()) {
                    std::cout << "\n[Внимание]: Машина уже остановлена (" 
                              << (tm->getStatus() == ExecutionStatus::HaltedByState ? "HaltedByState" : "HaltedNoRule")
                              << "). Задайте слово заново (пункт 1).\n";
                    break;
                }
                bool ok = tm->step();
                printState(*tm);
                if (!ok) {
                    std::cout << "[Останов]: Машина завершила работу. Статус: "
                              << (tm->getStatus() == ExecutionStatus::HaltedByState ? "HaltedByState" : "HaltedNoRule")
                              << '\n';
                }
                break;
            }

            case 3: {
                if (tm->isHalted()) {
                    std::cout << "\n[Внимание]: Машина уже остановлена. Задайте слово заново (пункт 1).\n";
                    break;
                }
                std::size_t max_steps = 1000;
                std::cout << "Лимит шагов [по умолчанию 1000]: ";
                std::string line = readLineTrimmed();
                if (!line.empty()) {
                    std::istringstream iss(line);
                    iss >> max_steps;
                }

                ExecutionStatus status = tm->run(max_steps);
                printState(*tm);

                std::cout << "Результат работы: ";
                switch (status) {
                    case ExecutionStatus::HaltedByState:    
                        std::cout << "Останов по переходу в терминальное состояние.\n"; 
                        break;
                    case ExecutionStatus::HaltedNoRule:     
                        std::cout << "Останов: правило для текущего состояния и символа не найдено.\n"; 
                        break;
                    case ExecutionStatus::MaxStepsExceeded: 
                        std::cout << "Превышена квота шагов (возможно зацикливание программы).\n"; 
                        break;
                    case ExecutionStatus::Running:          
                        std::cout << "Выполнение продолжается.\n"; 
                        break;
                }
                break;
            }

            case 4: {
                std::cout << "Текущая конфигурация автомата:\n"
                          << "  Стартовое состояние:  \"" << program.getInitialState() << "\"\n"
                          << "  Останавливающее:     \"" << program.getHaltState() << "\"\n"
                          << "  Символ пробела:       '" << alphabet.getBlank() << "'\n\n";
                std::cout << "Введите правило: <состояние> <символ> <новое_состояние> <запись> <сдвиг L/R/S>\n"
                          << "Пример: q0 1 q0 1 R\n> ";

                std::string cur_state, next_state;
                char read_sym{}, write_sym{}, dir_char{};

                if (std::cin >> cur_state >> read_sym >> next_state >> write_sym >> dir_char) {
                    Direction dir = Direction::Stay;
                    bool valid_dir = true;

                    if (dir_char == 'L' || dir_char == 'l') dir = Direction::Left;
                    else if (dir_char == 'R' || dir_char == 'r') dir = Direction::Right;
                    else if (dir_char == 'S' || dir_char == 's') dir = Direction::Stay;
                    else valid_dir = false;

                    if (!valid_dir) {
                        std::cout << "[Ошибка]: Недопустимое направление '" << dir_char << "'. Допустимы: L, R, S.\n";
                    } else {
                        try {
                            if (read_sym != alphabet.getBlank()) {
                                alphabet.addSymbol(read_sym);
                            }
                            if (write_sym != alphabet.getBlank()) {
                                alphabet.addSymbol(write_sym);
                            }

                            program.addRule(cur_state, read_sym, next_state, write_sym, dir);
                            std::cout << "[Успех]: Правило (" << cur_state << ", '" << read_sym 
                                      << "') -> (" << next_state << ", '" << write_sym << "', " << dir_char 
                                      << ") успешно сохранено.\n";
                        } catch (const std::exception& e) {
                            std::cout << "[Ошибка]: " << e.what() << '\n';
                        }
                    }
                } else {
                    std::cout << "[Ошибка]: Неверный формат ввода параметров правила.\n";
                }
                clearInputBuffer();
                break;
            }

            case 5: {
                const auto& rules = program.getAllRules();
                std::cout << "\n--- Зарегистрированные команды Тьюринга (" << rules.size() << ") ---\n";
                std::cout << "Стартовое состояние:    \"" << program.getInitialState() << "\"\n";
                std::cout << "Терминальное состояние: \"" << program.getHaltState() << "\"\n";
                
                if (rules.empty()) {
                    std::cout << "  (список команд пуст)\n";
                } else {
                    for (const auto& [key, action] : rules) {
                        char dir_c = (action.move_direction == Direction::Left) ? 'L' :
                                     (action.move_direction == Direction::Right) ? 'R' : 'S';
                        std::cout << "  (" << key.first << ", '" << key.second << "') -> ("
                                  << action.next_state << ", '" << action.write_symbol << "', " << dir_c << ")\n";
                    }
                }
                break;
            }

            case 6: {
                std::cout << "Введите символ для добавления в алфавит: ";
                char sym{};
                if (std::cin >> sym) {
                    try {
                        alphabet.addSymbol(sym);
                        std::cout << "[Успех]: Символ '" << sym << "' зарегистрирован в алфавите.\n";
                    } catch (const std::exception& e) {
                        std::cout << "[Ошибка]: " << e.what() << '\n';
                    }
                }
                clearInputBuffer();
                break;
            }

            case 7: {
                program.clear();
                program = Program("q_scan", "q_halt");
                program.addRule("q_scan", '1', "q_scan", '1', Direction::Right);
                program.addRule("q_scan", '_', "q_halt", '1', Direction::Stay);
                
                if (!alphabet.isValid('1')) {
                    alphabet.addSymbol('1');
                }

                tm = std::make_unique<TuringMachine>(alphabet, program);
                tm->reset("111", 0);

                std::cout << "[Успех]: Загружена программа унарного инкремента.\n"
                          << "Активные состояния автомата: старт = \"q_scan\", останов = \"q_halt\".\n"
                          << "Слово \"111\" установлено с позиции 0.\n";
                printState(*tm);
                break;
            }

            default:
                std::cout << "[Ошибка]: Неизвестный пункт меню. Выберите число от 0 до 7.\n";
                break;
        }
    }
    return 0;
}