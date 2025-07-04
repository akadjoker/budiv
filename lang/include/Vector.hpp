
#pragma once

template <typename T, typename Alloc = Allocator<T>> class Vector {
public:
    Vector(size_t initial_capacity = 0);
    ~Vector();
    Vector(const Vector &other);
    Vector &operator=(const Vector &other);
    Vector(Vector &&other) noexcept;
    Vector &operator=(Vector &&other) noexcept;

    void push_back(const T &value);
    void push_back(T &&value);
    T pop_back();
    template <typename... Args> void emplace_back(Args &&...args);

    T top();

    T &operator[](size_t index);
    const T &operator[](size_t index) const;

    size_t size() const;
    size_t capacity() const;
    void reserve(size_t new_capacity);
    T &back();
    bool empty() const;
    bool erase(size_t index);
    void clear();
    void resize(size_t new_capacity);

    const T *pointer() const;
    T *pointer();

    class Iterator;

    Iterator begin();
    Iterator end();

private:
    T *data;
    size_t sz;
    size_t cap;
    Alloc alloc;


    void destroy_elements();
    void copy_from(const Vector &other);
};

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector(size_t initial_capacity)
    : data(nullptr), sz(initial_capacity), cap(initial_capacity)
{
    if (initial_capacity > 0)
    {
        data = alloc.allocate(initial_capacity);
        for (size_t i = 0; i < sz; ++i)
        {
            const T &value = T();
            alloc.construct(&data[i], value);
        }
    }
}


template <typename T, typename Alloc> Vector<T, Alloc>::~Vector()
{
    destroy_elements();
    alloc.deallocate(data, cap);
}

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector(const Vector &other): data(nullptr), sz(0), cap(0)
{
    copy_from(other);
}

template <typename T, typename Alloc>
Vector<T, Alloc> &Vector<T, Alloc>::operator=(const Vector &other)
{
    if (this != &other)
    {
        destroy_elements();
        alloc.deallocate(data, cap);
        copy_from(other);
    }
    return *this;
}

template <typename T, typename Alloc>
Vector<T, Alloc>::Vector(Vector &&other) noexcept
    : data(other.data), sz(other.sz), cap(other.cap)
{
    other.data = nullptr;
    other.sz = 0;
    other.cap = 0;
}

template <typename T, typename Alloc>
Vector<T, Alloc> &Vector<T, Alloc>::operator=(Vector &&other) noexcept
{
    if (this != &other)
    {
        destroy_elements();
        alloc.deallocate(data, cap);

        data = other.data;
        sz = other.sz;
        cap = other.cap;

        other.data = nullptr;
        other.sz = 0;
        other.cap = 0;
    }
    return *this;
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::push_back(const T &value)
{
    if (sz == cap)
    {
        resize(CalculateCapacityGrow(cap, sz + 1));
    }
    alloc.construct(&data[sz++], value);
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::push_back(T &&value)
{
    if (sz == cap)
    {
        resize(CalculateCapacityGrow(cap, sz + 1));
    }
    alloc.construct(&data[sz++], std::move(value));
}

template <typename T, typename Alloc> T Vector<T, Alloc>::pop_back()
{
    DEBUG_BREAK_IF(sz == 0);
    T value = std::move(data[--sz]);
    alloc.destroy(&data[sz]);
    return value;
}

template <typename T, typename Alloc>
template <typename... Args>
void Vector<T, Alloc>::emplace_back(Args &&...args)
{
    if (sz == cap)
    {
        resize(CalculateCapacityGrow(cap, sz + 1));
    }
    alloc.construct(&data[sz++], std::forward<Args>(args)...);
}

template <typename T, typename Alloc> inline T Vector<T, Alloc>::top()
{
    DEBUG_BREAK_IF(sz == 0);
    return data[sz - 1];
}

template <typename T, typename Alloc>
T &Vector<T, Alloc>::operator[](size_t index)
{
    DEBUG_BREAK_IF(index > cap);
    return data[index];
}

template <typename T, typename Alloc>
inline const T &Vector<T, Alloc>::operator[](size_t index) const
{
    DEBUG_BREAK_IF(index > cap);
    return data[index];
}

template <typename T, typename Alloc> size_t Vector<T, Alloc>::size() const
{
    return sz;
}

template <typename T, typename Alloc> size_t Vector<T, Alloc>::capacity() const
{
    return cap;
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::reserve(size_t new_capacity)
{
    if (new_capacity > cap)
    {
        resize(CalculateCapacityGrow(cap, new_capacity));
    }
}

template <typename T, typename Alloc> T &Vector<T, Alloc>::back()
{
    DEBUG_BREAK_IF(sz == 0);
    return data[sz - 1];
}

template <typename T, typename Alloc> bool Vector<T, Alloc>::empty() const
{
    return sz == 0;
}

template <typename T, typename Alloc>
inline bool Vector<T, Alloc>::erase(size_t index)
{

    if (index >= sz)
    {
        return false;
    }
    if constexpr (!std::is_trivially_destructible<T>::value)
    {
        alloc.destroy(&data[index]);
    }
    for (size_t i = index; i < sz - 1; ++i)
    {
        data[i] = std::move(data[i + 1]);
    }
    --sz;
    return true;
}

template <typename T, typename Alloc> inline void Vector<T, Alloc>::clear()
{
    destroy_elements();
    sz = 0;
}

template <typename T, typename Alloc> void Vector<T, Alloc>::destroy_elements()
{
    if constexpr (!std::is_trivially_destructible<T>::value)
    {
        for (size_t i = 0; i < sz; ++i)
        {
            alloc.destroy(&data[i]);
        }
    }
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::resize(size_t new_capacity)
{
    T *new_data = alloc.allocate(new_capacity);
    if (data && sz > 0)
    {
        if constexpr (std::is_trivially_copyable<T>::value)
        {
            std::memcpy(new_data, data, sz * sizeof(T));
        }
        else
        {
            for (size_t i = 0; i < sz; ++i)
            {
                alloc.construct(&new_data[i], std::move(data[i]));
                alloc.destroy(&data[i]);
            }
        }
    }
    alloc.deallocate(data, cap);
    data = new_data;
    cap = new_capacity;
}

template <typename T, typename Alloc>
inline const T *Vector<T, Alloc>::pointer() const
{
    return data;
}

template <typename T, typename Alloc> inline T *Vector<T, Alloc>::pointer()
{
    return data;
}

template <typename T, typename Alloc>
void Vector<T, Alloc>::copy_from(const Vector &other)
{
    sz = other.sz;
    cap = other.cap;
    data = alloc.allocate(cap);
    if (other.data && sz > 0)
    {
        if constexpr (std::is_trivially_copyable<T>::value)
        {
            std::memcpy(data, other.data, sz * sizeof(T));
        }
        else
        {
            for (size_t i = 0; i < sz; ++i)
            {
                alloc.construct(&data[i], other.data[i]);
            }
        }
    }
}

template <typename T, typename Alloc> class Vector<T, Alloc>::Iterator {
public:
    Iterator(T *ptr): current(ptr) {}

    T &operator*() const { return *current; }

    Iterator &operator++()
    {
        ++current;
        return *this;
    }

    bool operator!=(const Iterator &other) const
    {
        return current != other.current;
    }

private:
    T *current;
};

template <typename T, typename Alloc>
typename Vector<T, Alloc>::Iterator Vector<T, Alloc>::begin()
{
    return Iterator(data);
}

template <typename T, typename Alloc>
typename Vector<T, Alloc>::Iterator Vector<T, Alloc>::end()
{
    return Iterator(data + sz);
}


template <typename T> class ValueArray {
private:
    T *data;

    size_t capacity;
    size_t size;

    static constexpr size_t DEFAULT_CAPACITY = 16;
    static constexpr double GROWTH_FACTOR = 1.5;


    void resize(size_t newCapacity)
    {
        T *newData = new T[newCapacity];


        for (size_t i = 0; i < size; ++i)
        {
            newData[i] = data[i];
        }


        delete[] data;


        data = newData;

        capacity = newCapacity;
    }


public:
    ValueArray(): capacity(DEFAULT_CAPACITY), size(0)
    {
        data = new T[capacity];
    }


    explicit ValueArray(size_t initialCapacity)
        : capacity(initialCapacity > 0 ? initialCapacity : DEFAULT_CAPACITY),
          size(0)
    {
        data = new T[capacity];
    }


    ~ValueArray() { delete[] data; }


    ValueArray(const ValueArray &other)
        : capacity(other.capacity), size(other.size)
    {
        data = new T[capacity];


        for (size_t i = 0; i < size; ++i)
        {
            data[i] = other.data[i];
        }
    }


    ValueArray &operator=(const ValueArray &other)
    {
        if (this != &other)
        {
            delete[] data;


            capacity = other.capacity;
            size = other.size;


            data = new T[capacity];


            for (size_t i = 0; i < size; ++i)
            {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void push_back(const T &value)
    {
        if (size >= capacity)
        {
            size_t newCapacity = static_cast<size_t>(capacity * GROWTH_FACTOR);
            resize(newCapacity);
        }

        data[size] = value;

        ++size;
    }


    void pop_back()
    {
        if (size > 0)
        {
            --size;
        }
    }


    void insert(size_t pos, const T &value)
    {
        if (pos > size) return;

        if (size >= capacity)
        {
            size_t newCapacity = static_cast<size_t>(capacity * GROWTH_FACTOR);
            resize(newCapacity);
        }


        for (size_t i = size; i > pos; --i)
        {
            data[i] = data[i - 1];
        }

        data[pos] = value;


        ++size;
    }


    bool remove(const T &value)
    {
        int index = find(value);
        if (index == -1) return false;

        return removeAt(static_cast<size_t>(index));
    }


    bool removeAt(size_t index)
    {
        if (index >= size) return false;


        for (size_t i = index; i < size - 1; ++i)
        {
            data[i] = data[i + 1];
        }

        --size;

        return true;
    }


    void clear() { size = 0; }


    T &operator[](size_t index) { return data[index]; }

    const T &operator[](size_t index) const { return data[index]; }


    T &at(size_t index)
    {

        DEBUG_BREAK_IF(index >= size);

        return data[index];
    }

    const T &at(size_t index) const
    {
        DEBUG_BREAK_IF(index >= size);
        return data[index];
    }

    size_t getSize() const { return size; }
    size_t getCapacity() const { return capacity; }
    bool empty() const { return size == 0; }


    T &front() { return data[0]; }
    const T &front() const { return data[0]; }

    T &back() { return data[size - 1]; }
    const T &back() const { return data[size - 1]; }


    void reserve(size_t newCapacity)
    {
        if (newCapacity > capacity)
        {
            resize(newCapacity);
        }
    }


    void shrink_to_fit()
    {
        if (capacity > size && size > 0)
        {
            resize(size);
        }
    }


    T *begin() { return data; }
    T *end() { return data + size; }
    const T *begin() const { return data; }
    const T *end() const { return data + size; }
};