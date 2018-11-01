#pragma once

#include <Windows.h>
#include <stdexcept>
#include <string>


class windows_shared_memory
{
    HANDLE handle_ = INVALID_HANDLE_VALUE;
    void* memory_ = nullptr;
    std::size_t size_ = 0;
    std::string name_;

    windows_shared_memory(const windows_shared_memory&) = delete;
    windows_shared_memory& operator=(const windows_shared_memory &) = delete;

    void swap(windows_shared_memory& other)
    {
        std::swap(handle_, other.handle_);
        std::swap(memory_, other.memory_);
        std::swap(name_, other.name_);
        std::swap(size_, other.size_);
    }
public:

    windows_shared_memory(windows_shared_memory&& other)
        : handle_(other.handle_), memory_(other.memory_), name_(other.name_), size_(other.size_)
    {
        other.handle_ = INVALID_HANDLE_VALUE;
        other.memory_ = nullptr;
        other.name_.clear();
        other.size_ = 0;
    }

    windows_shared_memory& operator=(windows_shared_memory && other)
    {
        if (this != &other)
        {
            windows_shared_memory(std::move(other)).swap(*this);
        }
        return *this;
    }

    enum access_mode_t {
        read_only,
        read_write
    };

    struct open_only_t {};
    struct create_only_t {};

    static const open_only_t open_only;
    static const create_only_t create_only;

    windows_shared_memory(create_only_t , const std::string& name, std::uint32_t size, access_mode_t access_mode)
        : name_(name)
    {
        this->create_file_mapping(size, access_mode);
        this->map(access_mode);
    }

    windows_shared_memory(open_only_t , const std::string& name, access_mode_t access_mode)
        : name_(name)
    {
        this->open_file_mapping(access_mode);
        this->map(access_mode);
    }

    ~windows_shared_memory()
    {
        this->unmap();
        if (handle_ && handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle_);
            handle_ = nullptr;
        }
    }

    void* get_address()
    {
        return memory_;
    }

    std::size_t get_size() const
    {
        return size_;
    }

private:
    void map(access_mode_t access_mode)
    {
        if (handle_)
        {
            memory_ = MapViewOfFile(handle_, access_mode == read_write ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
            if (!memory_)
            {
                DWORD err = GetLastError();
                throw std::runtime_error(std::string("system_error:") + std::to_string(err));
            }

            MEMORY_BASIC_INFORMATION info;
            if (!VirtualQuery(memory_, &info, sizeof(info)))
            {
                // Windows doesn't set an error code on this one,
                // it should only be a kernel memory error.
                throw std::runtime_error(std::string("system_error:VirtualQuery"));
            }
            size_ = info.RegionSize;
        }
    }

    void unmap()
    {
        if (memory_)
        {
            UnmapViewOfFile(memory_);
            memory_ = nullptr;
        }
    }

    void create_file_mapping(std::uint32_t size, access_mode_t access_mode)
    {
        HANDLE handle = CreateFileMappingA(INVALID_HANDLE_VALUE,
            NULL,
            access_mode == read_write ? PAGE_READWRITE : PAGE_READONLY,
            0,
            size,
            name_.c_str());

        DWORD err = GetLastError();
        if (handle && err == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(handle);
            throw std::runtime_error(std::string("system_error:") + std::to_string(err));
        }
        else if (!handle)
        {
            throw std::runtime_error(std::string("system_error:") + std::to_string(err));
        }
        else
        {
            handle_ = handle;
        }

    }

    void open_file_mapping(access_mode_t access_mode)
    {
        handle_ = OpenFileMappingA(access_mode == read_write ? FILE_MAP_WRITE : FILE_MAP_READ, FALSE, name_.c_str());
        if (!handle_)
        {
            DWORD err = GetLastError();

            throw  std::runtime_error(std::string("system_error:") + std::to_string(err));
        }
    }
};

__declspec(selectany) const windows_shared_memory::create_only_t windows_shared_memory::create_only;

__declspec(selectany) const windows_shared_memory::open_only_t windows_shared_memory::open_only;
