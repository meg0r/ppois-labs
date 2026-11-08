#pragma once
#include "GraphExceptions.hpp"
#include <iostream>
#include <iterator>
#include <utility>
#include <algorithm>
#include <cstddef>

namespace lab4 {

template <typename T>
class WirthGraph {
private:
    struct ArcNode;

    // Vertex node representation (Wirth's structure)
    struct VertexNode {
        T data;
        std::size_t in_degree = 0;
        std::size_t out_degree = 0;
        ArcNode* first_arc = nullptr;
        ArcNode* last_arc = nullptr;
        VertexNode* prev = nullptr;
        VertexNode* next = nullptr;

        explicit VertexNode(const T& val) : data(val) {}
    };

    // Arc node representation (Wirth's structure)
    struct ArcNode {
        VertexNode* target = nullptr;
        ArcNode* prev_arc = nullptr;
        ArcNode* next_arc = nullptr;

        explicit ArcNode(VertexNode* tgt) : target(tgt) {}
    };

    VertexNode* head_ = nullptr;
    VertexNode* tail_ = nullptr;
    std::size_t vertex_count_ = 0;
    std::size_t edge_count_ = 0;

    VertexNode* findVertexNode(const T& val) const noexcept {
        VertexNode* curr = head_;
        while (curr) {
            if (curr->data == val) return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    ArcNode* findArcNode(VertexNode* src, VertexNode* dst) const noexcept {
        if (!src || !dst) return nullptr;
        ArcNode* arc = src->first_arc;
        while (arc) {
            if (arc->target == dst) return arc;
            arc = arc->next_arc;
        }
        return nullptr;
    }

    void removeArcInternal(VertexNode* src, ArcNode* arc) noexcept {
        if (arc->prev_arc) arc->prev_arc->next_arc = arc->next_arc;
        else src->first_arc = arc->next_arc;

        if (arc->next_arc) arc->next_arc->prev_arc = arc->prev_arc;
        else src->last_arc = arc->prev_arc;

        src->out_degree--;
        arc->target->in_degree--;
        edge_count_--;
        delete arc;
    }

public:
    // Public type aliases (STL Container Concept compliance)
    using value_type = T;
    using edge_value_type = std::pair<T, T>;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // =========================================================================
    // 1. Bidirectional Vertex Iterator (VertexIterator)
    // =========================================================================
    template <bool IsConst>
    class VertexIteratorImpl {
        friend class WirthGraph;
    private:
        const WirthGraph* graph_ = nullptr;
        VertexNode* current_ = nullptr;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        VertexIteratorImpl() = default;
        VertexIteratorImpl(const WirthGraph* g, VertexNode* node) : graph_(g), current_(node) {}

        template <bool WasConst, typename = std::enable_if_t<IsConst && !WasConst>>
        VertexIteratorImpl(const VertexIteratorImpl<WasConst>& other)
            : graph_(other.graph_), current_(other.current_) {}

        reference operator*() const {
            if (!current_) throw InvalidIteratorException("Dereferencing end vertex iterator");
            return current_->data;
        }

        pointer operator->() const {
            if (!current_) throw InvalidIteratorException("Dereferencing end vertex iterator");
            return &(current_->data);
        }

        VertexIteratorImpl& operator++() {
            if (!current_) throw InvalidIteratorException("Incrementing end vertex iterator");
            current_ = current_->next;
            return *this;
        }

        VertexIteratorImpl operator++(int) {
            VertexIteratorImpl tmp = *this;
            ++(*this);
            return tmp;
        }

        VertexIteratorImpl& operator--() {
            if (!graph_) throw InvalidIteratorException("Decrementing uninitialized iterator");
            if (!current_) {
                if (!graph_->tail_) throw InvalidIteratorException("Decrementing end of empty graph");
                current_ = graph_->tail_;
            } else {
                if (!current_->prev) throw InvalidIteratorException("Decrementing begin iterator");
                current_ = current_->prev;
            }
            return *this;
        }

        VertexIteratorImpl operator--(int) {
            VertexIteratorImpl tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const VertexIteratorImpl& other) const noexcept {
            return current_ == other.current_ && graph_ == other.graph_;
        }
        bool operator!=(const VertexIteratorImpl& other) const noexcept {
            return !(*this == other);
        }
    };

    // =========================================================================
    // 2. Bidirectional Edge Iterator (EdgeIterator)
    // =========================================================================
    class EdgeIterator {
        friend class WirthGraph;
    private:
        const WirthGraph* graph_ = nullptr;
        VertexNode* v_curr_ = nullptr;
        ArcNode* a_curr_ = nullptr;
        mutable edge_value_type cached_pair_;

        void advanceToValid() noexcept {
            while (v_curr_ && !a_curr_) {
                v_curr_ = v_curr_->next;
                if (v_curr_) a_curr_ = v_curr_->first_arc;
            }
        }

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::pair<T, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        EdgeIterator() = default;
        EdgeIterator(const WirthGraph* g, VertexNode* v, ArcNode* a)
            : graph_(g), v_curr_(v), a_curr_(a) {
            advanceToValid();
        }

        reference operator*() const {
            if (!v_curr_ || !a_curr_) throw InvalidIteratorException("Dereferencing end edge iterator");
            cached_pair_ = std::make_pair(v_curr_->data, a_curr_->target->data);
            return cached_pair_;
        }

        pointer operator->() const {
            return &(**this);
        }

        EdgeIterator& operator++() {
            if (!v_curr_ || !a_curr_) throw InvalidIteratorException("Incrementing end edge iterator");
            a_curr_ = a_curr_->next_arc;
            advanceToValid();
            return *this;
        }

        EdgeIterator operator++(int) {
            EdgeIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        EdgeIterator& operator--() {
            if (!graph_) throw InvalidIteratorException("Decrementing uninitialized edge iterator");
            if (!v_curr_) {
                v_curr_ = graph_->tail_;
                while (v_curr_ && !v_curr_->last_arc) {
                    v_curr_ = v_curr_->prev;
                }
                if (!v_curr_) throw InvalidIteratorException("Decrementing begin of empty edge list");
                a_curr_ = v_curr_->last_arc;
            } else if (a_curr_->prev_arc) {
                a_curr_ = a_curr_->prev_arc;
            } else {
                VertexNode* prev_v = v_curr_->prev;
                while (prev_v && !prev_v->last_arc) {
                    prev_v = prev_v->prev;
                }
                if (!prev_v) throw InvalidIteratorException("Decrementing begin edge iterator");
                v_curr_ = prev_v;
                a_curr_ = v_curr_->last_arc;
            }
            return *this;
        }

        EdgeIterator operator--(int) {
            EdgeIterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const EdgeIterator& other) const noexcept {
            return graph_ == other.graph_ && v_curr_ == other.v_curr_ && a_curr_ == other.a_curr_;
        }
        bool operator!=(const EdgeIterator& other) const noexcept {
            return !(*this == other);
        }
    };

    // =========================================================================
    // 3. Bidirectional Incident Edge Iterator (IncidentEdgeIterator)
    // =========================================================================
    class IncidentEdgeIterator {
        friend class WirthGraph;
    private:
        VertexNode* source_ = nullptr;
        ArcNode* current_ = nullptr;
        mutable edge_value_type cached_pair_;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::pair<T, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        IncidentEdgeIterator() = default;
        IncidentEdgeIterator(VertexNode* src, ArcNode* arc) : source_(src), current_(arc) {}

        reference operator*() const {
            if (!source_ || !current_) throw InvalidIteratorException("Dereferencing end incident edge iterator");
            cached_pair_ = std::make_pair(source_->data, current_->target->data);
            return cached_pair_;
        }

        pointer operator->() const { return &(**this); }

        IncidentEdgeIterator& operator++() {
            if (!current_) throw InvalidIteratorException("Incrementing end incident edge iterator");
            current_ = current_->next_arc;
            return *this;
        }

        IncidentEdgeIterator operator++(int) {
            IncidentEdgeIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        IncidentEdgeIterator& operator--() {
            if (!source_) throw InvalidIteratorException("Decrementing uninitialized incident edge iterator");
            if (!current_) {
                if (!source_->last_arc) throw InvalidIteratorException("Decrementing empty incident edge list");
                current_ = source_->last_arc;
            } else {
                if (!current_->prev_arc) throw InvalidIteratorException("Decrementing begin incident edge iterator");
                current_ = current_->prev_arc;
            }
            return *this;
        }

        IncidentEdgeIterator operator--(int) {
            IncidentEdgeIterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const IncidentEdgeIterator& other) const noexcept {
            return source_ == other.source_ && current_ == other.current_;
        }
        bool operator!=(const IncidentEdgeIterator& other) const noexcept {
            return !(*this == other);
        }
    };

    // =========================================================================
    // 4. Bidirectional Adjacent Vertex Iterator (AdjacentVertexIterator)
    // =========================================================================
    template <bool IsConst>
    class AdjacentVertexIteratorImpl {
        friend class WirthGraph;
    private:
        VertexNode* source_ = nullptr;
        ArcNode* current_ = nullptr;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        AdjacentVertexIteratorImpl() = default;
        AdjacentVertexIteratorImpl(VertexNode* src, ArcNode* arc) : source_(src), current_(arc) {}

        template <bool WasConst, typename = std::enable_if_t<IsConst && !WasConst>>
        AdjacentVertexIteratorImpl(const AdjacentVertexIteratorImpl<WasConst>& other)
            : source_(other.source_), current_(other.current_) {}

        reference operator*() const {
            if (!current_) throw InvalidIteratorException("Dereferencing end adjacent vertex iterator");
            return current_->target->data;
        }

        pointer operator->() const {
            if (!current_) throw InvalidIteratorException("Dereferencing end adjacent vertex iterator");
            return &(current_->target->data);
        }

        AdjacentVertexIteratorImpl& operator++() {
            if (!current_) throw InvalidIteratorException("Incrementing end adjacent vertex iterator");
            current_ = current_->next_arc;
            return *this;
        }

        AdjacentVertexIteratorImpl operator++(int) {
            AdjacentVertexIteratorImpl tmp = *this;
            ++(*this);
            return tmp;
        }

        AdjacentVertexIteratorImpl& operator--() {
            if (!source_) throw InvalidIteratorException("Decrementing uninitialized adjacent iterator");
            if (!current_) {
                if (!source_->last_arc) throw InvalidIteratorException("Decrementing empty adjacent list");
                current_ = source_->last_arc;
            } else {
                if (!current_->prev_arc) throw InvalidIteratorException("Decrementing begin adjacent iterator");
                current_ = current_->prev_arc;
            }
            return *this;
        }

        AdjacentVertexIteratorImpl operator--(int) {
            AdjacentVertexIteratorImpl tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const AdjacentVertexIteratorImpl& other) const noexcept {
            return source_ == other.source_ && current_ == other.current_;
        }
        bool operator!=(const AdjacentVertexIteratorImpl& other) const noexcept {
            return !(*this == other);
        }
    };

    // =========================================================================
    // 5. Forward Adjacent Vertex Iterator (Variant 2.1.11 / 2.1.12 Requirement)
    // =========================================================================
    template <bool IsConst>
    class ForwardAdjacentIteratorImpl {
        friend class WirthGraph;
    private:
        ArcNode* current_ = nullptr;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        ForwardAdjacentIteratorImpl() = default;
        explicit ForwardAdjacentIteratorImpl(ArcNode* arc) : current_(arc) {}

        template <bool WasConst, typename = std::enable_if_t<IsConst && !WasConst>>
        ForwardAdjacentIteratorImpl(const ForwardAdjacentIteratorImpl<WasConst>& other)
            : current_(other.current_) {}

        reference operator*() const {
            if (!current_) throw InvalidIteratorException("Dereferencing end forward adjacent iterator");
            return current_->target->data;
        }

        pointer operator->() const {
            if (!current_) throw InvalidIteratorException("Dereferencing end forward adjacent iterator");
            return &(current_->target->data);
        }

        ForwardAdjacentIteratorImpl& operator++() {
            if (!current_) throw InvalidIteratorException("Incrementing end forward adjacent iterator");
            current_ = current_->next_arc;
            return *this;
        }

        ForwardAdjacentIteratorImpl operator++(int) {
            ForwardAdjacentIteratorImpl tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const ForwardAdjacentIteratorImpl& other) const noexcept {
            return current_ == other.current_;
        }
        bool operator!=(const ForwardAdjacentIteratorImpl& other) const noexcept {
            return !(*this == other);
        }
    };

    // Type aliases for iterators (regular, const, reverse, const_reverse)
    using vertex_iterator = VertexIteratorImpl<false>;
    using const_vertex_iterator = VertexIteratorImpl<true>;
    using reverse_vertex_iterator = std::reverse_iterator<vertex_iterator>;
    using const_reverse_vertex_iterator = std::reverse_iterator<const_vertex_iterator>;

    using iterator = vertex_iterator;
    using const_iterator = const_vertex_iterator;
    using reverse_iterator = reverse_vertex_iterator;
    using const_reverse_iterator = const_reverse_vertex_iterator;

    using edge_iterator = EdgeIterator;
    using const_edge_iterator = EdgeIterator;
    using reverse_edge_iterator = std::reverse_iterator<edge_iterator>;
    using const_reverse_edge_iterator = std::reverse_iterator<const_edge_iterator>;

    using incident_edge_iterator = IncidentEdgeIterator;
    using const_incident_edge_iterator = IncidentEdgeIterator;
    using reverse_incident_edge_iterator = std::reverse_iterator<incident_edge_iterator>;
    using const_reverse_incident_edge_iterator = std::reverse_iterator<const_incident_edge_iterator>;

    using adjacent_iterator = AdjacentVertexIteratorImpl<false>;
    using const_adjacent_iterator = AdjacentVertexIteratorImpl<true>;
    using reverse_adjacent_iterator = std::reverse_iterator<adjacent_iterator>;
    using const_reverse_adjacent_iterator = std::reverse_iterator<const_adjacent_iterator>;

    using forward_adjacent_iterator = ForwardAdjacentIteratorImpl<false>;
    using const_forward_adjacent_iterator = ForwardAdjacentIteratorImpl<true>;

    // =========================================================================
    // Lifecycle Management (Rule of Five)
    // =========================================================================
    WirthGraph() noexcept = default;

    WirthGraph(const WirthGraph& other) {
        for (auto it = other.begin_vertices(); it != other.end_vertices(); ++it) {
            addVertex(*it);
        }
        for (auto it = other.begin_edges(); it != other.end_edges(); ++it) {
            addEdge(it->first, it->second);
        }
    }

    WirthGraph(WirthGraph&& other) noexcept
        : head_(other.head_), tail_(other.tail_),
          vertex_count_(other.vertex_count_), edge_count_(other.edge_count_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.vertex_count_ = 0;
        other.edge_count_ = 0;
    }

    ~WirthGraph() noexcept {
        clear();
    }

    WirthGraph& operator=(const WirthGraph& other) {
        if (this != &other) {
            WirthGraph tmp(other);
            swap(tmp);
        }
        return *this;
    }

    WirthGraph& operator=(WirthGraph&& other) noexcept {
        if (this != &other) {
            clear();
            swap(other);
        }
        return *this;
    }

    void swap(WirthGraph& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(vertex_count_, other.vertex_count_);
        std::swap(edge_count_, other.edge_count_);
    }

    // =========================================================================
    // Core Container Methods
    // =========================================================================
    bool empty() const noexcept { return vertex_count_ == 0; }

    void clear() noexcept {
        VertexNode* v = head_;
        while (v) {
            ArcNode* a = v->first_arc;
            while (a) {
                ArcNode* next_a = a->next_arc;
                delete a;
                a = next_a;
            }
            VertexNode* next_v = v->next;
            delete v;
            v = next_v;
        }
        head_ = nullptr;
        tail_ = nullptr;
        vertex_count_ = 0;
        edge_count_ = 0;
    }

    size_type getVertexCount() const noexcept { return vertex_count_; }
    size_type getEdgeCount() const noexcept { return edge_count_; }

    // =========================================================================
    // Vertex and Edge Access and Manipulation
    // =========================================================================
    bool hasVertex(const T& val) const noexcept {
        return findVertexNode(val) != nullptr;
    }

    bool hasEdge(const T& from, const T& to) const noexcept {
        VertexNode* src = findVertexNode(from);
        VertexNode* dst = findVertexNode(to);
        return findArcNode(src, dst) != nullptr;
    }

    void addVertex(const T& val) {
        if (hasVertex(val)) {
            throw VertexAlreadyExistsException();
        }
        auto* node = new VertexNode(val);
        if (!head_) {
            head_ = tail_ = node;
        } else {
            tail_->next = node;
            node->prev = tail_;
            tail_ = node;
        }
        vertex_count_++;
    }

    void addEdge(const T& from, const T& to) {
        VertexNode* src = findVertexNode(from);
        VertexNode* dst = findVertexNode(to);
        if (!src || !dst) {
            throw VertexNotFoundException();
        }
        if (findArcNode(src, dst) != nullptr) {
            throw EdgeAlreadyExistsException();
        }
        auto* arc = new ArcNode(dst);
        if (!src->first_arc) {
            src->first_arc = src->last_arc = arc;
        } else {
            src->last_arc->next_arc = arc;
            arc->prev_arc = src->last_arc;
            src->last_arc = arc;
        }
        src->out_degree++;
        dst->in_degree++;
        edge_count_++;
    }

    void removeEdge(const T& from, const T& to) {
        VertexNode* src = findVertexNode(from);
        VertexNode* dst = findVertexNode(to);
        if (!src || !dst) {
            throw VertexNotFoundException();
        }
        ArcNode* arc = findArcNode(src, dst);
        if (!arc) {
            throw EdgeNotFoundException();
        }
        removeArcInternal(src, arc);
    }

    void removeVertex(const T& val) {
        VertexNode* target_v = findVertexNode(val);
        if (!target_v) {
            throw VertexNotFoundException();
        }

        // 1. Remove incoming arcs pointing to target_v
        VertexNode* curr = head_;
        while (curr) {
            if (curr != target_v) {
                ArcNode* arc = findArcNode(curr, target_v);
                if (arc) {
                    removeArcInternal(curr, arc);
                }
            }
            curr = curr->next;
        }

        // 2. Remove all outgoing arcs from target_v
        ArcNode* arc = target_v->first_arc;
        while (arc) {
            ArcNode* next_a = arc->next_arc;
            arc->target->in_degree--;
            edge_count_--;
            delete arc;
            arc = next_a;
        }

        // 3. Unlink vertex node
        if (target_v->prev) target_v->prev->next = target_v->next;
        else head_ = target_v->next;

        if (target_v->next) target_v->next->prev = target_v->prev;
        else tail_ = target_v->prev;

        delete target_v;
        vertex_count_--;
    }

    void removeVertex(const_vertex_iterator it) {
        if (it.graph_ != this || !it.current_) {
            throw InvalidIteratorException("Cannot remove vertex by invalid iterator");
        }
        removeVertex(*it);
    }

    void removeEdge(const_edge_iterator it) {
        if (it.graph_ != this || !it.v_curr_ || !it.a_curr_) {
            throw InvalidIteratorException("Cannot remove edge by invalid iterator");
        }
        removeArcInternal(it.v_curr_, it.a_curr_);
    }

    // =========================================================================
    // Vertex and Edge Degree Computation
    // =========================================================================
    size_type getVertexOutDegree(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return v->out_degree;
    }

    size_type getVertexInDegree(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return v->in_degree;
    }

    size_type getVertexDegree(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return v->out_degree + v->in_degree;
    }

    size_type getEdgeDegree(const T& from, const T& to) const {
        VertexNode* src = findVertexNode(from);
        VertexNode* dst = findVertexNode(to);
        if (!src || !dst) throw VertexNotFoundException();
        if (!findArcNode(src, dst)) throw EdgeNotFoundException();

        if (src == dst) {
            return (src->in_degree + src->out_degree >= 2) ? (src->in_degree + src->out_degree - 2) : 0;
        }
        size_type total = (src->in_degree + src->out_degree) + (dst->in_degree + dst->out_degree);
        return (total >= 2) ? (total - 2) : 0;
    }

    // =========================================================================
    // Iterator Factory Methods
    // =========================================================================
    vertex_iterator begin() noexcept { return vertex_iterator(this, head_); }
    vertex_iterator end() noexcept { return vertex_iterator(this, nullptr); }
    const_vertex_iterator begin() const noexcept { return const_vertex_iterator(this, head_); }
    const_vertex_iterator end() const noexcept { return const_vertex_iterator(this, nullptr); }
    const_vertex_iterator cbegin() const noexcept { return const_vertex_iterator(this, head_); }
    const_vertex_iterator cend() const noexcept { return const_vertex_iterator(this, nullptr); }

    reverse_vertex_iterator rbegin() noexcept { return reverse_vertex_iterator(end()); }
    reverse_vertex_iterator rend() noexcept { return reverse_vertex_iterator(begin()); }
    const_reverse_vertex_iterator rbegin() const noexcept { return const_reverse_vertex_iterator(end()); }
    const_reverse_vertex_iterator rend() const noexcept { return const_reverse_vertex_iterator(begin()); }
    const_reverse_vertex_iterator crbegin() const noexcept { return const_reverse_vertex_iterator(cend()); }
    const_reverse_vertex_iterator crend() const noexcept { return const_reverse_vertex_iterator(cbegin()); }

    vertex_iterator begin_vertices() noexcept { return begin(); }
    vertex_iterator end_vertices() noexcept { return end(); }
    const_vertex_iterator begin_vertices() const noexcept { return begin(); }
    const_vertex_iterator end_vertices() const noexcept { return end(); }
    const_vertex_iterator cbegin_vertices() const noexcept { return cbegin(); }
    const_vertex_iterator cend_vertices() const noexcept { return cend(); }
    reverse_vertex_iterator rbegin_vertices() noexcept { return rbegin(); }
    reverse_vertex_iterator rend_vertices() noexcept { return rend(); }
    const_reverse_vertex_iterator crbegin_vertices() const noexcept { return crbegin(); }
    const_reverse_vertex_iterator crend_vertices() const noexcept { return crend(); }

    edge_iterator begin_edges() const noexcept {
        return edge_iterator(this, head_, head_ ? head_->first_arc : nullptr);
    }
    edge_iterator end_edges() const noexcept {
        return edge_iterator(this, nullptr, nullptr);
    }
    const_edge_iterator cbegin_edges() const noexcept { return begin_edges(); }
    const_edge_iterator cend_edges() const noexcept { return end_edges(); }
    reverse_edge_iterator rbegin_edges() const noexcept { return reverse_edge_iterator(end_edges()); }
    reverse_edge_iterator rend_edges() const noexcept { return reverse_edge_iterator(begin_edges()); }
    const_reverse_edge_iterator crbegin_edges() const noexcept { return const_reverse_edge_iterator(cend_edges()); }
    const_reverse_edge_iterator crend_edges() const noexcept { return const_reverse_edge_iterator(cbegin_edges()); }

    incident_edge_iterator begin_incident_edges(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return incident_edge_iterator(v, v->first_arc);
    }
    incident_edge_iterator end_incident_edges(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return incident_edge_iterator(v, nullptr);
    }
    const_incident_edge_iterator cbegin_incident_edges(const T& val) const { return begin_incident_edges(val); }
    const_incident_edge_iterator cend_incident_edges(const T& val) const { return end_incident_edges(val); }
    reverse_incident_edge_iterator rbegin_incident_edges(const T& val) const {
        return reverse_incident_edge_iterator(end_incident_edges(val));
    }
    reverse_incident_edge_iterator rend_incident_edges(const T& val) const {
        return reverse_incident_edge_iterator(begin_incident_edges(val));
    }
    const_reverse_incident_edge_iterator crbegin_incident_edges(const T& val) const {
        return rbegin_incident_edges(val);
    }
    const_reverse_incident_edge_iterator crend_incident_edges(const T& val) const {
        return rend_incident_edges(val);
    }

    adjacent_iterator begin_adjacent(const T& val) {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return adjacent_iterator(v, v->first_arc);
    }
    adjacent_iterator end_adjacent(const T& val) {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return adjacent_iterator(v, nullptr);
    }
    const_adjacent_iterator begin_adjacent(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return const_adjacent_iterator(v, v->first_arc);
    }
    const_adjacent_iterator end_adjacent(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return const_adjacent_iterator(v, nullptr);
    }
    const_adjacent_iterator cbegin_adjacent(const T& val) const { return begin_adjacent(val); }
    const_adjacent_iterator cend_adjacent(const T& val) const { return end_adjacent(val); }

    reverse_adjacent_iterator rbegin_adjacent(const T& val) {
        return reverse_adjacent_iterator(end_adjacent(val));
    }
    reverse_adjacent_iterator rend_adjacent(const T& val) {
        return reverse_adjacent_iterator(begin_adjacent(val));
    }
    const_reverse_adjacent_iterator crbegin_adjacent(const T& val) const {
        return const_reverse_adjacent_iterator(cend_adjacent(val));
    }
    const_reverse_adjacent_iterator crend_adjacent(const T& val) const {
        return const_reverse_adjacent_iterator(cbegin_adjacent(val));
    }

    forward_adjacent_iterator begin_forward_adjacent(const T& val) {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return forward_adjacent_iterator(v->first_arc);
    }
    forward_adjacent_iterator end_forward_adjacent(const T& val) {
        if (!hasVertex(val)) throw VertexNotFoundException();
        return forward_adjacent_iterator(nullptr);
    }
    const_forward_adjacent_iterator cbegin_forward_adjacent(const T& val) const {
        VertexNode* v = findVertexNode(val);
        if (!v) throw VertexNotFoundException();
        return const_forward_adjacent_iterator(v->first_arc);
    }
    const_forward_adjacent_iterator cend_forward_adjacent(const T& val) const {
        if (!hasVertex(val)) throw VertexNotFoundException();
        return const_forward_adjacent_iterator(nullptr);
    }

    // =========================================================================
    // Comparison Operators (==, !=, <, >, <=, >=)
    // =========================================================================
    bool operator==(const WirthGraph& other) const noexcept {
        if (vertex_count_ != other.vertex_count_ || edge_count_ != other.edge_count_) {
            return false;
        }
        for (auto it = cbegin_vertices(); it != cend_vertices(); ++it) {
            if (!other.hasVertex(*it)) return false;
        }
        for (auto it = cbegin_edges(); it != cend_edges(); ++it) {
            if (!other.hasEdge(it->first, it->second)) return false;
        }
        return true;
    }

    bool operator!=(const WirthGraph& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const WirthGraph& other) const noexcept {
        if (vertex_count_ != other.vertex_count_) return vertex_count_ < other.vertex_count_;
        if (edge_count_ != other.edge_count_) return edge_count_ < other.edge_count_;
        return std::lexicographical_compare(cbegin_vertices(), cend_vertices(),
                                            other.cbegin_vertices(), other.cend_vertices());
    }

    bool operator>(const WirthGraph& other) const noexcept { return other < *this; }
    bool operator<=(const WirthGraph& other) const noexcept { return !(other < *this); }
    bool operator>=(const WirthGraph& other) const noexcept { return !(*this < other); }

    // =========================================================================
    // Stream Output using std::for_each and Iterators
    // =========================================================================
    friend std::ostream& operator<<(std::ostream& os, const WirthGraph& graph) {
        os << "Vertices (" << graph.vertex_count_ << "): [ ";
        std::for_each(graph.cbegin_vertices(), graph.cend_vertices(), [&os](const T& v) {
            os << v << " ";
        });
        os << "]\nEdges (" << graph.edge_count_ << "): [ ";
        std::for_each(graph.cbegin_edges(), graph.cend_edges(), [&os](const edge_value_type& e) {
            os << "(" << e.first << " -> " << e.second << ") ";
        });
        os << "]";
        return os;
    }
};

} // namespace lab4