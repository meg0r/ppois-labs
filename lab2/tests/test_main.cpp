#include <exception>
#include <iostream>

void runActorsTests();
void runDiagnosticsTests();
void runFinanceTests();
void runOrdersTests();
void runWarehouseTests();

int main() {
    std::cout << "Running service center tests...\n";

    try {
        runActorsTests();
    } catch (const std::exception& e) {
        std::cerr << "[-] Error in Actors: " << e.what() << '\n';
        return 1;
    }

    try {
        runDiagnosticsTests();
    } catch (const std::exception& e) {
        std::cerr << "[-] Error in Diagnostics: " << e.what() << '\n';
        return 1;
    }

    try {
        runFinanceTests();
    } catch (const std::exception& e) {
        std::cerr << "[-] Error in Finance: " << e.what() << '\n';
        return 1;
    }

    try {
        runOrdersTests();
    } catch (const std::exception& e) {
        std::cerr << "[-] Error in Orders: " << e.what() << '\n';
        return 1;
    }

    try {
        runWarehouseTests();
    } catch (const std::exception& e) {
        std::cerr << "[-] Error in Warehouse: " << e.what() << '\n';
        return 1;
    }

    std::cout << "\nAll test suites successfully passed!\n";
    return 0;
}