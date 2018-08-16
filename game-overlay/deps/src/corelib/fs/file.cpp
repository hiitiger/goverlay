#include "stable.h"
#include "file.h"
#include "path.h"
namespace Storm
{
    File::File(const std::wstring& file)
        : file_(INVALID_HANDLE_VALUE)
    {
        fullPath_ = Path(file).fullPath();
    }

    File::~File()
    {
        close();
    }

    const std::wstring File::fileName() const
    {
        return Path(fullPath_).fileName();
    }

    const std::wstring File::fullPath() const
    {
        return fullPath_;
    }

    bool File::isValid() const
    {
        return file_ != INVALID_HANDLE_VALUE;
    }

    bool File::open(OpenMode openMode)
    {
        if (fullPath_.empty())
        {
            return false;
        }

        DWORD accessRight = 0;
        if (openMode & File::ReadOnly)
        {
            accessRight |= GENERIC_READ;
        }
        if (openMode & File::WriteOnly)
        {
            accessRight |= GENERIC_WRITE;
        }

        SECURITY_ATTRIBUTES securityAtts = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };

        DWORD creationDisp = (openMode & File::WriteOnly) ? OPEN_ALWAYS : OPEN_EXISTING;

        file_ = ::CreateFileW(fullPath_.c_str(), accessRight, FILE_SHARE_READ | FILE_SHARE_WRITE, &securityAtts, creationDisp, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (file_ == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            (void)err;
            return false;
        }

        if (openMode & File::Truncate)
        {
            setSize(0);
        }

        if (openMode & File::Append)
        {
            seekEnd();
        }

        return true;
    }

    void File::close()
    {
        if (file_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(file_);
            file_ = INVALID_HANDLE_VALUE;
        }
    }

    bool File::setSize(int64_t sz)
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        int64_t currentPos = pos();

        if (seek(sz) && ::SetEndOfFile(file_))
        {
            seek(std::min(currentPos, sz));
            return true;
        }

        seek(currentPos);
        return false;
    }

    int64_t File::size() const
    {
        WIN32_FILE_ATTRIBUTE_DATA attributes = { 0 };
        if (::GetFileAttributesExW(fullPath().c_str(), GetFileExInfoStandard, (void*)&attributes))
        {
            return (((int64_t)attributes.nFileSizeHigh) << 32) | attributes.nFileSizeLow;
        }
        return -1;
    }

    int64_t File::pos() const
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return -1;
        }

        LARGE_INTEGER currentPos;
        LARGE_INTEGER move;
        move.QuadPart = 0;
        if (::SetFilePointerEx(file_, move, &currentPos, FILE_CURRENT))
        {
            return static_cast<int64_t>(currentPos.QuadPart);
        }
        return -1;
    }

    bool File::seek(int64_t offset)
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        LARGE_INTEGER currentPos;
        LARGE_INTEGER move;
        move.QuadPart = offset;
        if (!::SetFilePointerEx(file_, move, &currentPos, FILE_BEGIN))
        {
            DWORD err = ::GetLastError();
            (void)err;
            return false;
        }

        return true;
    }

    bool File::seekEnd()
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        return INVALID_SET_FILE_POINTER != ::SetFilePointer(file_, 0, 0, FILE_END);
    }

    bool File::atEnd() const
    {
        return isValid() 
            && pos() == size();
    }

    int32_t File::read(uint8_t* data, int32_t len)
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return -1;
        }

        static const DWORD maxBlockSize = 32 * 1024 * 1024;
        int32_t totalRead = 0;
        DWORD bytesToRead = DWORD(len);

        do {
            DWORD blockSize = std::min(bytesToRead, maxBlockSize);
            DWORD bytesRead = 0;

            if (!::ReadFile(file_, data + totalRead, blockSize, &bytesRead, nullptr))
            {
                if (totalRead == 0)
                {
                    DWORD err = ::GetLastError();
                    (void)err;
                    return -1;
                }
                else
                {
                    break;
                }
            }

            if (bytesRead == 0)
            {
                break;
            }

            totalRead += bytesRead;
            bytesToRead -= bytesRead;
        } while (totalRead < len);

        return totalRead;

    }

    int32_t File::write(const uint8_t* data, int32_t len)
    {
        if (file_ == INVALID_HANDLE_VALUE)
        {
            return -1;
        }

        static const DWORD maxBlockSize = 32 * 1024 * 1024;
        int32_t totalWritten = 0;
        DWORD bytesToWrite = DWORD(len);
        do
        {
            DWORD blockSize = std::min(bytesToWrite, maxBlockSize);
            DWORD bytesWritten = 0;

            if (!::WriteFile(file_, data + totalWritten, blockSize, &bytesWritten, nullptr))
            {
                if (totalWritten == 0)
                {
                    DWORD err = ::GetLastError();
                    (void)err;
                    return -1;
                }
                else
                {
                    break;
                }
            }

            if (bytesWritten == 0)
            {
                break;
            }

            totalWritten += bytesWritten;
            bytesToWrite -= bytesWritten;

        } while (totalWritten < len);

        return totalWritten;
    }

    Buffer File::readAll()
    {
        Buffer data;
        int32_t size = (int32_t)(this->size() - this->pos());
        if (size > 0)
        {
            uint8_t* ptr = data.allocToAdd( (uint32_t) size);
            int32_t r = read(ptr, size);
            DAssert(r == size);

            if (r < size)
                data.remove(r, (uint32_t)(size - r));
        }
        return data;
    }

    Buffer File::readLine()
    {
        const std::int32_t maxLength = 65536;
        std::int32_t readLength = 0;
        std::unique_ptr<uint8_t[]> buffer{ new uint8_t[maxLength] };

        while (readLength < maxLength)
        {
            uint8_t c = 0;
            std::int32_t readsize = read(&c, 1);
            if (readsize <= 0)
                break;
            buffer[readLength++] = c;
            if (c == '\n')
                break;
        }
        Buffer line;
        line.assign(buffer.get(), readLength);
        return line;
    }

    std::vector<Buffer> File::readLines()
    {
        std::vector<Buffer> lines;
        if (isValid())
        {
            while (!atEnd())
            {
                Buffer line = readLine();
                lines.push_back(line);
            }
        }
        
        return lines;
    }

    int32_t File::writeLine(const Buffer& buffer)
    {
        int32_t w = 0;
        if (buffer.size()>0)
        {
            w = write(buffer.data(), buffer.size());
            if (w >= 0)
            {
                w += write((const uint8_t*)"\r\n", 2);
            }
        }
        return w;
    }

    int32_t File::writeLines(const std::vector<Buffer>& buffers)
    {
        int32_t wa = 0;
        for (const auto& buffer : buffers)
        {
            int32_t a = writeLine(buffer);
            if (a < 0)
                break;
            wa += a;
        }
        return wa;
    }

    bool File::rename(const std::wstring& fileName)
    {
        close();

        if (!fullPath_.empty())
        {
            Path path = Path(fullPath_);
            std::wstring newFile = (path.absoluteDir() / fileName).fullPath();
            return  MoveFileW(fullPath_.c_str(), newFile.c_str()) != 0;
        }
        else
            return false;
    }

    bool File::exists(const std::wstring& file)
    {
        WIN32_FILE_ATTRIBUTE_DATA findData;
        return 0 != ::GetFileAttributesEx(file.c_str(), GetFileExInfoStandard, &findData);
    }


    bool File::remove(const std::wstring& file)
    {
        return DeleteFileW(file.c_str()) != 0;
    }

    bool File::copy(const std::wstring& src, const std::wstring& dst)
    {
        return CopyFileW(src.c_str(), dst.c_str(), TRUE) != 0;
    }

    bool File::copy(const std::wstring& src, const std::wstring& dst, bool overwrite)
    {
        return CopyFileW(src.c_str(), dst.c_str(), overwrite ? FALSE : TRUE) != 0;
    }

    bool File::move(const std::wstring& src, const std::wstring& dst)
    {
        return MoveFileW(src.c_str(), dst.c_str()) != 0;
    }

    bool File::move(const std::wstring& src, const std::wstring& dst, bool overwrite)
    {
        if (overwrite)
            return copy(src, dst, true) && remove(src);
        else
            return copy(src, dst) && remove(src);
    }

}
