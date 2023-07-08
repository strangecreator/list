#pragma once
#include <cstddef>
#include <memory>
#include <stdexcept>

template <typename Mylist, typename ValueType>
class List_Iterator_Base {
    using size_type_ = typename Mylist::size_type;

  public:
    using value_type = typename Mylist::value_type;
    using difference_type = typename Mylist::difference_type;
    using pointer = ValueType*;
    using reference = ValueType&;
    using iterator_category = std::bidirectional_iterator_tag;

    using BaseNode = typename Mylist::BaseNode;

  private:
    BaseNode* ptr_;

  public:
    List_Iterator_Base(BaseNode* ptr)
        : ptr_(ptr) {}

    template <typename U = ValueType>
    List_Iterator_Base(
        const typename Mylist::iterator& it,
        std::enable_if_t<std::is_const_v<U>>* /*unused*/ = nullptr)
        : ptr_(it.ptr_) {}

    reference operator*() const noexcept {
        return static_cast<typename Mylist::Node*>(ptr_)->object;
    }
    pointer operator->() const noexcept {
        return &(static_cast<typename Mylist::Node*>(ptr_)->object);
    }

    List_Iterator_Base& operator+=(difference_type offset) noexcept {
        if (offset > 0) {
            while (offset != 0) {
                ptr_ = ptr_->next;
                --offset;
            }
        } else if (offset < 0) {
            while (offset != 0) {
                ptr_ = ptr_->prev;
                ++offset;
            }
        }
        return *this;
    }
    List_Iterator_Base& operator-=(const difference_type offset) noexcept {
        *this += -offset;
        return *this;
    }

    List_Iterator_Base& operator++() noexcept {
        *this += 1;
        return *this;
    }
    List_Iterator_Base& operator--() noexcept {
        *this -= 1;
        return *this;
    }

    List_Iterator_Base operator++(int) noexcept {
        List_Iterator_Base Tmp = *this;
        ++*this;
        return Tmp;
    }
    List_Iterator_Base operator--(int) noexcept {
        List_Iterator_Base Tmp = *this;
        --*this;
        return Tmp;
    }

    bool operator==(const List_Iterator_Base& it) const noexcept {
        return ptr_ == it.ptr_;
    }
    bool operator!=(const List_Iterator_Base& it) const noexcept {
        return !(*this == it);
    }

  private:
    BaseNode* get_pointer() {
        return ptr_;
    }

    friend Mylist;
    friend List_Iterator_Base<Mylist, const ValueType>;
};

template <typename Mylist, typename ValueType>
List_Iterator_Base<Mylist, ValueType> operator+(
    List_Iterator_Base<Mylist, ValueType> it,
    typename List_Iterator_Base<Mylist, ValueType>::difference_type offset) {
    it += offset;
    return it;
}

template <typename Mylist, typename ValueType>
List_Iterator_Base<Mylist, ValueType> operator-(
    List_Iterator_Base<Mylist, ValueType> it,
    typename List_Iterator_Base<Mylist, ValueType>::difference_type offset) {
    it -= offset;
    return it;
}

template <typename Mylist, typename ValueType>
List_Iterator_Base<Mylist, ValueType> operator+(
    typename List_Iterator_Base<Mylist, ValueType>::difference_type offset,
    List_Iterator_Base<Mylist, ValueType> it) {
    it += offset;
    return it;
}

template <typename T, typename Alloc = std::allocator<T>>
class List : private Alloc { // empty base optimization
  public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = int;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    // iterators
    using iterator = List_Iterator_Base<List<T, Alloc>, T>;
    using const_iterator = List_Iterator_Base<List<T, Alloc>, const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // allocator types
    using allocator_type = Alloc;
    using AllocTraits = std::allocator_traits<allocator_type>;

  private:
    struct BaseNode {
        BaseNode* prev = nullptr;
        BaseNode* next = nullptr;
    };

    struct Node : BaseNode {
        T object;

        Node(BaseNode* prev, BaseNode* next, const T& object)
            : BaseNode{prev, next}, object(object) {}

        template <typename... Args>
        Node(BaseNode* prev, BaseNode* next, Args&&... args)
            : BaseNode{prev, next}, object(std::forward<Args>(args)...) {}
    };

    BaseNode _fake_node;
    size_type _size = 0;

  public:
    using AllocNode = typename AllocTraits::template rebind_alloc<Node>;
    using AllocNodeTraits = std::allocator_traits<AllocNode>;

    // constructors
    List() {
        _fake_node.prev = &_fake_node;
        _fake_node.next = &_fake_node;
    }
    explicit List(const allocator_type& alloc)
        : allocator_type(alloc) {
        _fake_node.prev = &_fake_node;
        _fake_node.next = &_fake_node;
    }
    explicit List(size_t count)
        : List() {
        while (count != 0) {
            emplace_back();
            --count;
        }
    }
    List(size_t count, const allocator_type& alloc)
        : List(alloc) {
        while (count != 0) {
            emplace_back();
            --count;
        }
    }
    List(size_t count, const T& value)
        : List() {
        while (count != 0) {
            push_back(value);
            --count;
        }
    }
    List(size_t count, const T& value, const allocator_type& alloc)
        : List(alloc) {
        while (count != 0) {
            push_back(value);
            --count;
        }
    }

    // copying operators
    List(const List& other)
        : List(AllocTraits::select_on_container_copy_construction(static_cast<allocator_type>(other))) {
        for (const T& value : other) {
            push_back(value);
        }
    }
    List& operator=(const List& other) {
        List temp(other);
        swap(temp);
        if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
            static_cast<allocator_type&>(*this) = static_cast<const allocator_type&>(other);
        }
        return *this;
    }

    ~List() {
        clear();
    }

    // other methods
    allocator_type get_allocator() const noexcept {
        return static_cast<allocator_type>(*this);
    }

    void push_back(const T& value) {
        insert(cend(), value);
    }
    void push_front(const T& value) {
        insert(cbegin(), value);
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        emplace(cend(), std::forward<Args>(args)...);
    }
    template <typename... Args>
    void emplace_front(Args&&... args) {
        emplace(cbegin(), std::forward<Args>(args)...);
    }

    void pop_back() noexcept {
        erase(--cend());
    }
    void pop_front() noexcept {
        erase(cbegin());
    }

    template <typename... Args>
    void emplace(const_iterator it, Args&&... args) {
        AllocNode alloc = get_node_allocator();
        BaseNode* node = it.get_pointer();
        BaseNode* prev = node->prev;
        Node* ptr = AllocNodeTraits::allocate(alloc, 1);
        try {
            AllocNodeTraits::construct(alloc, ptr, prev, node, std::forward<Args>(args)...);
        } catch (...) {
            AllocNodeTraits::deallocate(alloc, ptr, 1);
            throw;
        }
        prev->next = static_cast<BaseNode*>(ptr);
        node->prev = static_cast<BaseNode*>(ptr);
        ++_size;
    }

    void insert(const_iterator it, const T& value) {
        AllocNode alloc = get_node_allocator();
        BaseNode* node = it.get_pointer();
        BaseNode* prev = node->prev;
        Node* ptr = AllocNodeTraits::allocate(alloc, 1);
        try {
            AllocNodeTraits::construct(alloc, ptr, prev, node, value);
        } catch (...) {
            AllocNodeTraits::deallocate(alloc, ptr, 1);
            throw;
        }
        prev->next = static_cast<BaseNode*>(ptr);
        node->prev = static_cast<BaseNode*>(ptr);
        ++_size;
    }
    void erase(const_iterator it) noexcept {
        AllocNode alloc = get_node_allocator();
        Node* node = static_cast<Node*>(it.get_pointer());
        BaseNode* prev = node->prev;
        BaseNode* next = node->next;
        AllocNodeTraits::destroy(alloc, node);
        AllocNodeTraits::deallocate(alloc, node, 1);
        prev->next = next;
        next->prev = prev;
        --_size;
    }

    size_type size() const noexcept {
        return _size;
    }

    iterator begin() noexcept {
        return iterator(_fake_node.next);
    }
    const_iterator cbegin() const noexcept {
        return const_iterator(const_cast<BaseNode*>(_fake_node.next));  //NOLINT
    }
    const_iterator begin() const noexcept {
        return cbegin();
    }

    iterator end() noexcept {
        return iterator(&_fake_node);
    }
    const_iterator cend() const noexcept {
        return const_iterator(const_cast<BaseNode*>(&_fake_node));  //NOLINT
    }
    const_iterator end() const noexcept {
        return cend();
    }

    auto rbegin() noexcept {
        return std::reverse_iterator<iterator>(end());
    }
    auto crbegin() const noexcept {
        return std::reverse_iterator<const_iterator>(end());
    }
    auto rbegin() const noexcept {
        return std::reverse_iterator<const_iterator>(end());
    }

    auto rend() noexcept {
        return std::reverse_iterator<iterator>(begin());
    }
    auto crend() const noexcept {
        return std::reverse_iterator<const_iterator>(begin());
    }
    auto rend() const noexcept {
        return std::reverse_iterator<const_iterator>(begin());
    }

  private:
    AllocNode get_node_allocator() const noexcept {
        return AllocNode(get_allocator());
    }

    void clear() {
        while (size() != 0) {
            pop_back();
        }
    }

    void swap(List& other) {
        std::swap(_size, other._size);
        std::swap(_fake_node, other._fake_node);
        std::swap(_fake_node.prev->next, other._fake_node.prev->next);
        std::swap(_fake_node.next->prev, other._fake_node.next->prev);
        if constexpr (AllocTraits::propagate_on_container_swap::value) {
            std::swap(static_cast<allocator_type>(*this), static_cast<allocator_type>(other));
        }
    }

    template <typename Mylist, typename ValueType>
    friend class List_Iterator_Base;
};
