#include "Alphabet.hpp"
#include "Head.hpp"
#include "Program.hpp"
#include "Tape.hpp"
#include "TuringMachine.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int total_tests = 0;
int passed_tests = 0;

#define TEST_ASSERT(cond)                                                      \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "  [FAIL] " << #cond << " (Line " << __LINE__ << ")\n"; \
            return false;                                                      \
        }                                                                      \
    } while (false)

#define RUN_TEST(fn)                                                           \
    do {                                                                       \
        ++total_tests;                                                         \
        std::cout << "TEST " << total_tests << ": " << #fn << " ... ";        \
        if (fn()) {                                                            \
            std::cout << "PASSED\n";                                           \
            ++passed_tests;                                                    \
        } else {                                                               \
            std::cout << "FAILED\n";                                           \
        }                                                                      \
    } while (false)

// Тест 1: Валидация и неизменность blank-символа в Alphabet
bool testAlphabetValidation() {
    Alphabet alphabet{'_'};
    alphabet.addSymbols("01");

    TEST_ASSERT(alphabet.getBlank() == '_');
    TEST_ASSERT(alphabet.isValid('_'));
    TEST_ASSERT(alphabet.isValid('0'));
    TEST_ASSERT(alphabet.isValid('1'));
    TEST_ASSERT(!alphabet.isValid('2'));
    TEST_ASSERT(!alphabet.isValid(' '));
    return true;
}

// Тест 2: Движение каретки Head (инкремент, декремент, установка позиции)
bool testHeadMovement() {
    Head head(0);
    TEST_ASSERT(head.getPosition() == 0);

    ++head;
    TEST_ASSERT(head.getPosition() == 1);

    --head;
    --head;
    TEST_ASSERT(head.getPosition() == -1);

    head.setPosition(105);
    TEST_ASSERT(head.getPosition() == 105);
    return true;
}

// Тест 3: Разреженность ленты Tape и удаление ячеек при записи blank
bool testTapeSparseStorage() {
    Alphabet alphabet{'_'};
    alphabet.addSymbols("ab");
    Tape tape(alphabet);

    TEST_ASSERT(tape.empty());
    TEST_ASSERT(tape.read(-10) == '_');

    tape.write(-2, 'a');
    tape.write(3, 'b');
    TEST_ASSERT(!tape.empty());
    TEST_ASSERT(tape.toString() == "a____b");

    // Запись blank стирает ячейку из внутренней структуры
    tape.write(-2, '_');
    TEST_ASSERT(tape.toString() == "b");

    tape.write(3, '_');
    TEST_ASSERT(tape.empty());
    return true;
}

// Тест 4: Загрузка строки в Tape и проверка диапазона координат getBounds()
bool testTapeLoadStringAndBounds() {
    Alphabet alphabet{'_'};
    alphabet.addSymbols("xyz");
    Tape tape(alphabet);

    tape.loadString("xyz", -1);
    TEST_ASSERT(tape.read(-1) == 'x');
    TEST_ASSERT(tape.read(0) == 'y');
    TEST_ASSERT(tape.read(1) == 'z');

    auto bounds = tape.getBounds();
    TEST_ASSERT(bounds.first == -1);
    TEST_ASSERT(bounds.second == 1);
    return true;
}

// Тест 5: Добавление, извлечение и удаление правил в Program
bool testProgramRulesManagement() {
    Program program("q0", "halt");
    program.addRule("q0", '0', "q1", '1', Direction::Right);

    auto action = program.getAction("q0", '0');
    TEST_ASSERT(action.has_value());
    TEST_ASSERT(action->next_state == "q1");
    TEST_ASSERT(action->write_symbol == '1');
    TEST_ASSERT(action->move_direction == Direction::Right);

    TEST_ASSERT(!program.getAction("q0", '1').has_value());

    TEST_ASSERT(program.removeRule("q0", '0'));
    TEST_ASSERT(!program.getAction("q0", '0').has_value());
    return true;
}

// Тест 6: Десериализация таблицы переходов Program из потока ввода
bool testProgramLoadFromStream() {
    Program program("q0", "halt");
    std::string rules_data = 
        "# Комментарий\n"
        "q0 0 q0 1 R\n"
        "q0 1 halt 0 S\n";

    std::istringstream iss(rules_data);
    std::size_t loaded = program.loadFromStream(iss);

    TEST_ASSERT(loaded == 2);
    TEST_ASSERT(program.size() == 2);

    auto act = program.getAction("q0", '1');
    TEST_ASSERT(act.has_value());
    TEST_ASSERT(act->next_state == "halt");
    TEST_ASSERT(act->move_direction == Direction::Stay);
    return true;
}

// Тест 7: Корректное выполнение алгоритма унарного инкремента
bool testTuringMachineUnaryIncrement() {
    Alphabet alphabet{'_'};
    alphabet.addSymbol('1');

    Program program("q_scan", "q_halt");
    program.addRule("q_scan", '1', "q_scan", '1', Direction::Right);
    program.addRule("q_scan", '_', "q_halt", '1', Direction::Stay);

    TuringMachine tm(alphabet, program);
    tm.reset("111", 0);

    ExecutionStatus status = tm.run(100);

    TEST_ASSERT(status == ExecutionStatus::HaltedByState);
    TEST_ASSERT(tm.getStepCount() == 4);
    TEST_ASSERT(tm.getTape().toString() == "1111");
    TEST_ASSERT(tm.isHalted());
    return true;
}

// Тест 8: Останов HaltedNoRule при отсутствии правила перехода
bool testTuringMachineHaltedNoRule() {
    Alphabet alphabet{'_'};
    alphabet.addSymbols("01");

    Program program("q0", "q_halt");
    program.addRule("q0", '0', "q_halt", '0', Direction::Stay);

    TuringMachine tm(alphabet, program);
    tm.reset("1", 0); // Для '1' правил нет

    bool step_success = tm.step();

    TEST_ASSERT(!step_success);
    TEST_ASSERT(tm.getStatus() == ExecutionStatus::HaltedNoRule);
    TEST_ASSERT(tm.getStepCount() == 0); // Счетчик не инкрементируется
    TEST_ASSERT(tm.isHalted());
    return true;
}

// Тест 9: Ограничение квоты шагов (MaxStepsExceeded) и возобновление
bool testTuringMachineStepLimitAndResume() {
    Alphabet alphabet{'_'};
    alphabet.addSymbol('1');

    // Бесконечный цикл сдвига вправо
    Program program("loop", "halt");
    program.addRule("loop", '1', "loop", '1', Direction::Right);

    TuringMachine tm(alphabet, program);
    tm.reset("111111", 0);

    ExecutionStatus st1 = tm.run(3);
    TEST_ASSERT(st1 == ExecutionStatus::MaxStepsExceeded);
    TEST_ASSERT(tm.getStepCount() == 3);
    TEST_ASSERT(!tm.isHalted());
    TEST_ASSERT(tm.getStatus() == ExecutionStatus::Running);

    // Возобновление работы с места паузы
    ExecutionStatus st2 = tm.run(2);
    TEST_ASSERT(st2 == ExecutionStatus::MaxStepsExceeded);
    TEST_ASSERT(tm.getStepCount() == 5);
    TEST_ASSERT(tm.getHead().getPosition() == 5);
    return true;
}

// Тест 10: Сброс состояния через reset() и повторный запуск
bool testTuringMachineReset() {
    Alphabet alphabet{'_'};
    alphabet.addSymbol('1');

    Program program("q0", "halt");
    program.addRule("q0", '1', "halt", '1', Direction::Stay);

    TuringMachine tm(alphabet, program);
    tm.reset("1", 0);
    TEST_ASSERT(tm.run() == ExecutionStatus::HaltedByState);

    // Повторный сброс машины с новым словом
    tm.reset("11", 2);
    TEST_ASSERT(tm.getStatus() == ExecutionStatus::Running);
    TEST_ASSERT(tm.getStepCount() == 0);
    TEST_ASSERT(tm.getHead().getPosition() == 2);
    TEST_ASSERT(!tm.isHalted());
    return true;
}

} // namespace

int main() {
    std::cout << "========================================\n";
    std::cout << "     Turing Machine Unit Tests (x10)    \n";
    std::cout << "========================================\n";

    RUN_TEST(testAlphabetValidation);
    RUN_TEST(testHeadMovement);
    RUN_TEST(testTapeSparseStorage);
    RUN_TEST(testTapeLoadStringAndBounds);
    RUN_TEST(testProgramRulesManagement);
    RUN_TEST(testProgramLoadFromStream);
    RUN_TEST(testTuringMachineUnaryIncrement);
    RUN_TEST(testTuringMachineHaltedNoRule);
    RUN_TEST(testTuringMachineStepLimitAndResume);
    RUN_TEST(testTuringMachineReset);

    std::cout << "========================================\n";
    std::cout << "SUMMARY: " << passed_tests << " / " << total_tests << " tests passed.\n";
    std::cout << "========================================\n";

    return (passed_tests == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}