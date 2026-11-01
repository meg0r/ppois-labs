#include "CLI.hpp"
#include "Exceptions.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        CLI applicationInterface;
        applicationInterface.run();
    } catch (const RepairCompanyException& domainException) {
        std::cerr << "Domain business logic error: "
                  << domainException.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& standardException) {
        std::cerr << "Unexpected application failure: "
                  << standardException.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}