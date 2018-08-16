#pragma once
namespace Storm
{

    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(uint32_t sz);
        Buffer(const uint8_t* data, uint32_t sz);
        Buffer(const Buffer& other);
        Buffer(Buffer&& other);

        ~Buffer();

        Buffer& operator = (const Buffer& other);
        Buffer& operator = (Buffer&& other);

        void reserve(uint32_t sz);
        void assign(const uint8_t* data, uint32_t sz);

        void clear();
        void destroy();

        uint8_t* data();
        const uint8_t* data() const;

        uint32_t capacity() const;
        uint32_t size() const;

        uint8_t* allocToAdd(uint32_t sz);

        void add(const uint8_t* data, uint32_t sz);
        uint32_t remove(uint32_t offset, uint32_t sz);


    private:
        void swap(Buffer& other)
        {
            std::swap(data_, other.data_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
        }

        uint8_t* data_ = nullptr;
        uint32_t size_ = 0;
        uint32_t capacity_ = 0;
    };

    inline Buffer::Buffer(uint32_t sz)
    {
        reserve(sz);
    }

    inline Buffer::Buffer(const uint8_t* data, uint32_t sz)
    {
        assign(data, sz);
    }

    inline Buffer::Buffer(const Buffer& other)
    {
        this->assign(other.data_, other.size_);
    }

    inline Buffer::Buffer(Buffer&& other)
        : data_(other.data_)
        , size_(other.size_)
        , capacity_(other.capacity_)
    {
        other.data_ = 0;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    inline Buffer::~Buffer()
    {
        destroy();
    }

    inline Buffer& Buffer::operator=(const Buffer& other)
    {
        if (this != &other)
        {
            Buffer(other).swap(*this);
        }
        return *this;
    }

    inline Buffer& Buffer::operator=(Buffer&& other)
    {
        if (this != &other)
        {
            Buffer(std::move(other)).swap(*this);
        }
        return *this;
    }

    inline void Buffer::reserve(uint32_t sz)
    {
        if (capacity_ < sz)
        {
            uint8_t* data = (uint8_t*)malloc(sz);
            memset(data, 0, sz);

            if (this->size_ > 0) {
                std::memcpy((void*)data, this->data_, this->size_);
            }

            if (this->data_) {
                free(this->data_);
            }

            this->data_ = data;
            this->capacity_ = sz;
        }
    }

    inline void Buffer::assign(const uint8_t* data, uint32_t sz)
    {
        clear();
        reserve(sz);
        memcpy_s(this->data_, sz, data, sz);
        this->size_ = sz;
    }


    inline void Buffer::clear()
    {
        size_ = 0;
    }

    inline void Buffer::destroy()
    {
        if (data_)
        {
            free(data_);
            data_ = nullptr;
        }
        size_ = 0;
        capacity_ = 0;
    }

    inline uint8_t* Buffer::data()
    {
        return this->data_;
    }

    inline const uint8_t* Buffer::data() const
    {
        return this->data_;
    }

    inline uint32_t Buffer::capacity() const
    {
        return this->capacity_;
    }

    inline uint32_t Buffer::size() const
    {
        return this->size_;
    }

    inline uint8_t* Buffer::allocToAdd(uint32_t sz)
    {
        reserve(this->size_ + sz);
        auto ptr = this->data_ + this->size_;
        this->size_ += sz;
        return ptr;
    }

    inline void Buffer::add(const uint8_t* data, uint32_t sz)
    {
        reserve(this->size_ + sz);
        auto ptr = this->data_ + this->size_;
        std::memcpy((void*)ptr, (void*)data, sz);
        this->size_ += sz;
    }

    inline uint32_t Buffer::remove(uint32_t offset, uint32_t sz)
    {
        if (offset > this->size_)
        {
            return 0;
        }
        if (offset + sz > this->size_)
        {
            sz = this->size_ - offset;
        }
        uint32_t moved = this->size_ - (offset + sz);
        if (moved > 0)
        {
            std::memmove(this->data_ + offset, this->data_ + offset + sz, moved);
        }
        this->size_ -= sz;
        return sz;
    }


}