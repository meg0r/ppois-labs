#include <gtest/gtest.h>
#include "Sorting.hpp"
#include "Student.hpp"
#include "WirthGraph.hpp"
#include <vector>
#include <sstream>

using namespace lab4;

// =============================================================================
// Тесты Задания 1: Сортировки (Heapsort & Stooge sort)
// =============================================================================
TEST(SortingTest, HeapSortArrayAndVector) {
    int arr[] = {9, 1, 5, 3, 7, 2, 8, 6, 4, 0};
    heapSort(arr);
    for (int i = 0; i < 10; ++i) EXPECT_EQ(arr[i], i);

    std::vector<double> vec = {3.14, 1.41, 2.71, 0.57};
    heapSort(vec, std::greater<>());
    EXPECT_DOUBLE_EQ(vec[0], 3.14);
    EXPECT_DOUBLE_EQ(vec[3], 0.57);

    std::vector<int> empty_vec;
    EXPECT_NO_THROW(heapSort(empty_vec));
}

TEST(SortingTest, StoogeSortCustomObjects) {
    std::vector<Student> students = {
        Student("B", 2, 7.5),
        Student("A", 1, 9.0),
        Student("C", 3, 7.5),
        Student("D", 0, 6.0)
    };
    stoogeSort(students);
    EXPECT_EQ(students[0].getName(), "D");
    EXPECT_EQ(students[1].getName(), "B");
    EXPECT_EQ(students[2].getName(), "C");
    EXPECT_EQ(students[3].getName(), "A");

    int raw_arr[] = {5, 2, 9, 1};
    stoogeSort(raw_arr);
    EXPECT_EQ(raw_arr[0], 1);
    EXPECT_EQ(raw_arr[3], 9);
}

TEST(StudentTest, ComparisonOperatorsAndStream) {
    Student s1("Alice", 1, 8.0);
    Student s2("Bob", 2, 9.0);
    EXPECT_TRUE(s1 < s2);
    EXPECT_TRUE(s2 > s1);
    EXPECT_TRUE(s1 <= s2);
    EXPECT_TRUE(s2 >= s1);
    EXPECT_TRUE(s1 != s2);
    EXPECT_TRUE(s1 == Student("Alice", 1, 8.0));

    std::ostringstream oss;
    oss << s1;
    EXPECT_NE(oss.str().find("Alice"), std::string::npos);
}

// =============================================================================
// Тесты Задания 2: Ориентированный граф (Структура Вирта)
// =============================================================================
TEST(WirthGraphTest, BasicVertexAndEdgeOperations) {
    WirthGraph<int> g;
    EXPECT_TRUE(g.empty());
    EXPECT_EQ(g.getVertexCount(), 0u);
    EXPECT_EQ(g.getEdgeCount(), 0u);

    g.addVertex(1);
    g.addVertex(2);
    g.addVertex(3);
    EXPECT_FALSE(g.empty());
    EXPECT_EQ(g.getVertexCount(), 3u);
    EXPECT_TRUE(g.hasVertex(2));
    EXPECT_FALSE(g.hasVertex(99));

    EXPECT_THROW(g.addVertex(1), VertexAlreadyExistsException);

    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 3);
    EXPECT_EQ(g.getEdgeCount(), 3u);
    EXPECT_TRUE(g.hasEdge(1, 2));
    EXPECT_FALSE(g.hasEdge(2, 1)); // Граф ориентированный!

    EXPECT_THROW(g.addEdge(1, 2), EdgeAlreadyExistsException);
    EXPECT_THROW(g.addEdge(1, 99), VertexNotFoundException);
}

TEST(WirthGraphTest, DegreesCalculation) {
    WirthGraph<std::string> g;
    g.addVertex("A");
    g.addVertex("B");
    g.addVertex("C");
    g.addEdge("A", "B");
    g.addEdge("A", "C");
    g.addEdge("B", "C");

    EXPECT_EQ(g.getVertexOutDegree("A"), 2u);
    EXPECT_EQ(g.getVertexInDegree("A"), 0u);
    EXPECT_EQ(g.getVertexDegree("A"), 2u);

    EXPECT_EQ(g.getVertexOutDegree("C"), 0u);
    EXPECT_EQ(g.getVertexInDegree("C"), 2u);
    EXPECT_EQ(g.getVertexDegree("C"), 2u);

    // Степень ребра A->B: deg(A)=2, deg(B)=2 => 2 + 2 - 2 = 2
    EXPECT_EQ(g.getEdgeDegree("A", "B"), 2u);

    EXPECT_THROW(g.getVertexDegree("Z"), VertexNotFoundException);
    EXPECT_THROW(g.getEdgeDegree("B", "A"), EdgeNotFoundException);
}

TEST(WirthGraphTest, RemovalsByValueAndByIterator) {
    WirthGraph<int> g;
    for (int i = 1; i <= 4; ++i) g.addVertex(i);
    g.addEdge(1, 2);
    g.addEdge(2, 3);
    g.addEdge(3, 1);
    g.addEdge(4, 2);

    // Удаление ребра по итератору
    auto e_it = g.begin_edges(); // (1, 2)
    g.removeEdge(e_it);
    EXPECT_FALSE(g.hasEdge(1, 2));
    EXPECT_EQ(g.getEdgeCount(), 3u);

    // Удаление вершины по итератору (вершина 2 имеет входящие и исходящие дуги)
    auto v_it = g.begin_vertices();
    ++v_it; // указывает на 2
    g.removeVertex(v_it);
    EXPECT_FALSE(g.hasVertex(2));
    EXPECT_EQ(g.getVertexCount(), 3u);
    EXPECT_EQ(g.getEdgeCount(), 1u); // осталось только ребро (3, 1)

    EXPECT_THROW(g.removeVertex(999), VertexNotFoundException);
    EXPECT_THROW(g.removeEdge(1, 3), EdgeNotFoundException);
}

TEST(WirthGraphTest, AllIteratorsForwardBidirectionalAndReverse) {
    WirthGraph<int> g;
    g.addVertex(10);
    g.addVertex(20);
    g.addVertex(30);
    g.addEdge(10, 20);
    g.addEdge(10, 30);
    g.addEdge(20, 30);

    // Reverse vertex iterator
    std::vector<int> rev_v(g.rbegin_vertices(), g.rend_vertices());
    ASSERT_EQ(rev_v.size(), 3u);
    EXPECT_EQ(rev_v[0], 30);
    EXPECT_EQ(rev_v[2], 10);

    // Reverse edge iterator
    std::vector<std::pair<int, int>> rev_e(g.rbegin_edges(), g.rend_edges());
    ASSERT_EQ(rev_e.size(), 3u);
    EXPECT_EQ(rev_e[0], std::make_pair(20, 30));
    EXPECT_EQ(rev_e[2], std::make_pair(10, 20));

    // Incident edges (прямой и обратный)
    std::vector<std::pair<int, int>> inc(g.rbegin_incident_edges(10), g.rend_incident_edges(10));
    ASSERT_EQ(inc.size(), 2u);
    EXPECT_EQ(inc[0], std::make_pair(10, 30));
    EXPECT_EQ(inc[1], std::make_pair(10, 20));

    // Adjacent vertices (двунаправленный и однонаправленный Forward)
    std::vector<int> adj_rev(g.crbegin_adjacent(10), g.crend_adjacent(10));
    EXPECT_EQ(adj_rev[0], 30);
    EXPECT_EQ(adj_rev[1], 20);

    std::vector<int> adj_fwd(g.cbegin_forward_adjacent(10), g.cend_forward_adjacent(10));
    EXPECT_EQ(adj_fwd[0], 20);
    EXPECT_EQ(adj_fwd[1], 30);

    // Проверка исключений при выходе за границы итераторов
    EXPECT_THROW(*g.end_vertices(), InvalidIteratorException);
    EXPECT_THROW(++g.end_vertices(), InvalidIteratorException);
    EXPECT_THROW(--g.begin_vertices(), InvalidIteratorException);
}

TEST(WirthGraphTest, CopyMoveComparisonAndStreamOutput) {
    WirthGraph<int> g1;
    g1.addVertex(1);
    g1.addVertex(2);
    g1.addEdge(1, 2);

    // Конструктор копирования и оператор сравнения
    WirthGraph<int> g2(g1);
    EXPECT_EQ(g1, g2);
    EXPECT_LE(g1, g2);
    EXPECT_GE(g1, g2);

    g2.addVertex(3);
    EXPECT_NE(g1, g2);
    EXPECT_LT(g1, g2);
    EXPECT_GT(g2, g1);

    // Оператор присваивания и перемещения
    WirthGraph<int> g3;
    g3 = g1;
    EXPECT_EQ(g3, g1);

    WirthGraph<int> g4(std::move(g3));
    EXPECT_EQ(g4, g1);
    EXPECT_TRUE(g3.empty());

    // Вывод в поток через operator<< (с использованием std::for_each)
    std::ostringstream oss;
    oss << g1;
    std::string out = oss.str();
    EXPECT_NE(out.find("1 -> 2"), std::string::npos);
}