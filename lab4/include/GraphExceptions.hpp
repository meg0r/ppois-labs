#pragma once
#include <stdexcept>
#include <string>

namespace lab4 {

class GraphException : public std::runtime_error {
public:
    explicit GraphException(const std::string& message)
        : std::runtime_error(message) {}
};

class VertexNotFoundException : public GraphException {
public:
    explicit VertexNotFoundException(const std::string& message = "Vertex not found in graph")
        : GraphException(message) {}
};

class VertexAlreadyExistsException : public GraphException {
public:
    explicit VertexAlreadyExistsException(const std::string& message = "Vertex already exists in graph")
        : GraphException(message) {}
};

class EdgeNotFoundException : public GraphException {
public:
    explicit EdgeNotFoundException(const std::string& message = "Edge not found in graph")
        : GraphException(message) {}
};

class EdgeAlreadyExistsException : public GraphException {
public:
    explicit EdgeAlreadyExistsException(const std::string& message = "Edge already exists in graph")
        : GraphException(message) {}
};

class InvalidIteratorException : public GraphException {
public:
    explicit InvalidIteratorException(const std::string& message = "Invalid or dereferenced end iterator")
        : GraphException(message) {}
};

} // namespace lab4