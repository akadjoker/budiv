template <typename T, int N> class Array 
{
    T data[N];
    int size_ = 0;
public:

    // Array() { data = new T[N]; }
    // ~Array() { delete[] data; }


    void push(const T& value)
    {
        DEBUG_BREAK_IF(size_ == N);
    
        data[size_++] = value;
    }
    void push_back(const T& value) { push(value); }
    T pop()
    {
        DEBUG_BREAK_IF(size_ == 0);
        return data[--size_];
    }
    T& back()
    {
        DEBUG_BREAK_IF(size_ == 0);
        return data[size_ - 1];
    }
    T& pop_back() { return back(); }
    void clear() { size_ = 0; }
    int size() const { return size_; }
    int capacity() const { return N; }
    bool empty() const { return size_ == 0; }
    T& operator[](int idx)
    {
        DEBUG_BREAK_IF(idx < 0 || idx >= size_);
        return data[idx];
    }
};
