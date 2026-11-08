#include "Sorting.hpp"
#include "Student.hpp"
#include "WirthGraph.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace lab4;

void demoSorting() {
    std::cout << "\n=== Task 1 Demonstration: Heapsort & Stooge sort ===\n";

    // 1. Raw C-array of built-in type (int) sorted with Heapsort
    int arr[] = {42, 15, 8, 99, 23, 4, 16};
    std::cout << "Original C-array (int): ";
    for (int x : arr) std::cout << x << " ";
    heapSort(arr);
    std::cout << "\nAfter Heapsort:         ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n\n";

    // 2. std::vector of custom Student class sorted with Stooge sort
    std::vector<Student> students = {
        Student("Ivanov", 103, 8.5),
        Student("Petrov", 101, 9.2),
        Student("Sidorov", 102, 7.8),
        Student("Alekseev", 104, 8.5)
    };
    std::cout << "Original std::vector<Student>:\n";
    for (const auto& s : students) std::cout << "  " << s << "\n";

    stoogeSort(students);
    std::cout << "After Stooge sort (ascending by GPA):\n";
    for (const auto& s : students) std::cout << "  " << s << "\n";
}

void runGraphMenu() {
    WirthGraph<std::string> graph;
    int choice = -1;

    while (choice != 0) {
        std::cout << "\n=== Directed Graph Menu (Wirth Structure) ===\n"
                  << "1. Add vertex\n"
                  << "2. Add directed edge (arc)\n"
                  << "3. Remove vertex\n"
                  << "4. Remove edge\n"
                  << "5. Get vertex and edge degrees\n"
                  << "6. Print graph (operator<< using std::for_each)\n"
                  << "7. Traverse adjacent vertices (Forward & Reverse iterators)\n"
                  << "8. Run sorting demo (Task 1)\n"
                  << "0. Exit\n"
                  << "Your choice: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        try {
            if (choice == 1) {
                std::string v;
                std::cout << "Enter vertex name: ";
                std::cin >> v;
                graph.addVertex(v);
                std::cout << "Vertex added.\n";
            } else if (choice == 2) {
                std::string u, v;
                std::cout << "Enter source and destination vertices: ";
                std::cin >> u >> v;
                graph.addEdge(u, v);
                std::cout << "Directed edge " << u << " -> " << v << " added.\n";
            } else if (choice == 3) {
                std::string v;
                std::cout << "Enter vertex name to remove: ";
                std::cin >> v;
                graph.removeVertex(v);
                std::cout << "Vertex removed.\n";
            } else if (choice == 4) {
                std::string u, v;
                std::cout << "Enter source and destination vertices of the edge: ";
                std::cin >> u >> v;
                graph.removeEdge(u, v);
                std::cout << "Directed edge removed.\n";
            } else if (choice == 5) {
                std::string u;
                std::cout << "Enter vertex name: ";
                std::cin >> u;
                std::cout << "Total degree: " << graph.getVertexDegree(u)
                          << " (out-degree: " << graph.getVertexOutDegree(u)
                          << ", in-degree: " << graph.getVertexInDegree(u) << ")\n";
            } else if (choice == 6) {
                std::cout << graph << "\n";
            } else if (choice == 7) {
                std::string u;
                std::cout << "Enter vertex name: ";
                std::cin >> u;
                std::cout << "Forward adjacent traversal: ";
                for (auto it = graph.begin_forward_adjacent(u); it != graph.end_forward_adjacent(u); ++it) {
                    std::cout << *it << " ";
                }
                std::cout << "\nReverse bidirectional adjacent traversal: ";
                for (auto it = graph.rbegin_adjacent(u); it != graph.rend_adjacent(u); ++it) {
                    std::cout << *it << " ";
                }
                std::cout << "\n";
            } else if (choice == 8) {
                demoSorting();
            }
        } catch (const GraphException& ex) {
            std::cerr << "[Graph Error]: " << ex.what() << "\n";
        }
    }
}

int main() {
    demoSorting();
    runGraphMenu();
    return 0;
}