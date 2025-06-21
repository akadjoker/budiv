template <typename T, typename Alloc = Allocator<T>> 
class Queue {
public:
    Queue();
    ~Queue();
    Queue(const Queue &other);
    Queue &operator=(const Queue &other);
    Queue(Queue &&other) noexcept;
    Queue &operator=(Queue &&other) noexcept;

    template <typename... Args> void emplace(Args &&...args);

    void push(const T &value);
    void push(T &&value);
    T pop();
    T &front();
    T &back();
    bool empty() const;
    size_t size() const;

private:
    T *data;
    size_t sz;
    size_t cap;
    size_t head;
    size_t tail;
    size_t mask;  // Para otimização de módulo: mask = cap - 1
    Alloc alloc;

    void grow();
    void destroy_elements();
    void copy_from(const Queue &other);
    
    // Garantir que capacidade seja sempre potência de 2
    static constexpr size_t round_up_power_of_2(size_t n) {
        if (n <= 1) return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        if constexpr (sizeof(size_t) > 4) {
            n |= n >> 32;
        }
        return n + 1;
    }
};

template <typename T, typename Alloc>
Queue<T, Alloc>::Queue(): data(nullptr), sz(0), cap(0), head(0), tail(0), mask(0) {
    // Alocação completamente lazy
     resize(32); 
}

template <typename T, typename Alloc> 
Queue<T, Alloc>::~Queue() {
    destroy_elements();
    if (data) {
        alloc.deallocate(data, cap);
    }
}

template <typename T, typename Alloc>
Queue<T, Alloc>::Queue(const Queue &other)
    : data(nullptr), sz(0), cap(0), head(0), tail(0), mask(0) {
    copy_from(other);
}

template <typename T, typename Alloc>
Queue<T, Alloc> &Queue<T, Alloc>::operator=(const Queue &other) {
    if (this != &other) {
        destroy_elements();
        if (data) {
            alloc.deallocate(data, cap);
        }
        data = nullptr;
        sz = cap = head = tail = mask = 0;
        copy_from(other);
    }
    return *this;
}

template <typename T, typename Alloc>
Queue<T, Alloc>::Queue(Queue &&other) noexcept
    : data(other.data), sz(other.sz), cap(other.cap), 
      head(other.head), tail(other.tail), mask(other.mask), alloc(std::move(other.alloc)) {
    other.data = nullptr;
    other.sz = 0;
    other.cap = 0;
    other.head = 0;
    other.tail = 0;
    other.mask = 0;
}

template <typename T, typename Alloc>
Queue<T, Alloc> &Queue<T, Alloc>::operator=(Queue &&other) noexcept {
    if (this != &other) {
        destroy_elements();
        if (data) {
            alloc.deallocate(data, cap);
        }

        data = other.data;
        sz = other.sz;
        cap = other.cap;
        head = other.head;
        tail = other.tail;
        mask = other.mask;
        alloc = std::move(other.alloc);

        other.data = nullptr;
        other.sz = 0;
        other.cap = 0;
        other.head = 0;
        other.tail = 0;
        other.mask = 0;
    }
    return *this;
}

template <typename T, typename Alloc> 
inline void Queue<T, Alloc>::push(const T &value) {
    if (__builtin_expect(sz == cap, 0)) {  // Branch prediction hint
        grow();
    }
    
    // Construção direta baseada no tipo
    if constexpr (std::is_trivially_copyable_v<T>) {
        data[tail] = value;
    } else {
        alloc.construct(&data[tail], value);
    }
    
    tail = (tail + 1) & mask;  // Usar bitwise AND em vez de módulo
    ++sz;
}

template <typename T, typename Alloc> 
inline void Queue<T, Alloc>::push(T &&value) {
    if (__builtin_expect(sz == cap, 0)) {
        grow();
    }
    
    alloc.construct(&data[tail], std::move(value));
    tail = (tail + 1) & mask;
    ++sz;
}

template <typename T, typename Alloc>
template <typename... Args>
inline void Queue<T, Alloc>::emplace(Args &&...args) {
    if (__builtin_expect(sz == cap, 0)) {
        grow();
    }
    
    alloc.construct(&data[tail], std::forward<Args>(args)...);
    tail = (tail + 1) & mask;
    ++sz;
}

template <typename T, typename Alloc> 
inline T Queue<T, Alloc>::pop() {
    DEBUG_BREAK_IF(sz == 0);
    
    T *elem = &data[head];
    head = (head + 1) & mask;
    --sz;
    
    if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
        return *elem;
    } else {
        T value = std::move(*elem);
        alloc.destroy(elem);
        return value;
    }
}

template <typename T, typename Alloc> 
inline T &Queue<T, Alloc>::front() {
    DEBUG_BREAK_IF(sz == 0);
    return data[head];
}

template <typename T, typename Alloc> 
inline T &Queue<T, Alloc>::back() {
    DEBUG_BREAK_IF(sz == 0);
    return data[(tail - 1) & mask];
}

template <typename T, typename Alloc> 
inline bool Queue<T, Alloc>::empty() const {
    return sz == 0;
}

template <typename T, typename Alloc> 
inline size_t Queue<T, Alloc>::size() const {
    return sz;
}

template <typename T, typename Alloc>
void Queue<T, Alloc>::grow() {
    size_t new_cap = cap == 0 ? 64 : cap * 2;  // Capacidade inicial maior
    T *new_data = alloc.allocate(new_cap);
    
    if (sz > 0) {
        if constexpr (std::is_trivially_copyable_v<T>) {
            // Otimização para tipos triviais - usar memcpy sempre que possível
            size_t head_to_end = cap - head;
            if (head_to_end >= sz) {
                // Todos os elementos estão contíguos
                std::memcpy(new_data, data + head, sz * sizeof(T));
            } else {
                // Elementos divididos - duas cópias
                std::memcpy(new_data, data + head, head_to_end * sizeof(T));
                std::memcpy(new_data + head_to_end, data, (sz - head_to_end) * sizeof(T));
            }
        } else {
            // Para tipos não-triviais
            for (size_t i = 0; i < sz; ++i) {
                size_t old_idx = (head + i) & mask;
                alloc.construct(&new_data[i], std::move(data[old_idx]));
                alloc.destroy(&data[old_idx]);
            }
        }
    }
    
    if (data) {
        alloc.deallocate(data, cap);
    }
    
    data = new_data;
    cap = new_cap;
    mask = cap - 1;  // Funciona porque cap é sempre potência de 2
    head = 0;
    tail = sz;
}

template <typename T, typename Alloc> 
void Queue<T, Alloc>::destroy_elements() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
        if (data && sz > 0) {
            for (size_t i = 0; i < sz; ++i) {
                alloc.destroy(&data[(head + i) & mask]);
            }
        }
    }
}

template <typename T, typename Alloc>
void Queue<T, Alloc>::copy_from(const Queue &other) {
    if (other.sz == 0) {
        return;  // Mantém estado inicial
    }

    sz = other.sz;
    cap = other.cap;
    mask = other.mask;
    head = 0;
    tail = sz;
    data = alloc.allocate(cap);

    if constexpr (std::is_trivially_copyable_v<T>) {
        size_t head_to_end = other.cap - other.head;
        if (head_to_end >= sz) {
            std::memcpy(data, other.data + other.head, sz * sizeof(T));
        } else {
            std::memcpy(data, other.data + other.head, head_to_end * sizeof(T));
            std::memcpy(data + head_to_end, other.data, (sz - head_to_end) * sizeof(T));
        }
    } else {
        for (size_t i = 0; i < sz; ++i) {
            alloc.construct(&data[i], other.data[(other.head + i) & other.mask]);
        }
    }
}
