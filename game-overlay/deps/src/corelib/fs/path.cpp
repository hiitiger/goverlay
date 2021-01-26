#include "stable.h"
#include "path.h"

namespace Storm
{
    const wchar_t k_natvieDelimiter = '\\';

    std::wstring toNativeFullPath(const std::wstring& path)
    {
        std::wstring fullPath;
        std::vector<wchar_t> buffer;
        buffer.resize(MAX_PATH);
        wchar_t *fileName = 0;
        DWORD retLen = GetFullPathNameW((wchar_t*)path.c_str(), (DWORD)buffer.size(), buffer.data(), &fileName);
        if (retLen > (DWORD)MAX_PATH)
        {
            buffer.resize(retLen);
            retLen = GetFullPathNameW((wchar_t*)path.c_str(), (DWORD)buffer.size(), buffer.data(), &fileName);
        }
        if (retLen != 0)
            fullPath.assign(buffer.data(), retLen);
        if (!path.empty() && path.at(path.size() - 1) == ' ')
            fullPath.push_back(' ');
        if (fullPath != L"/" && fullPath.size() > 0 && fullPath[fullPath.size() - 1] == k_natvieDelimiter)
            fullPath = fullPath.substr(0, fullPath.size() - 1);
        return fullPath;

    }

    Path::Path()
    {

    }

    Path::Path(const std::wstring& file)
    {
        fullPath_ = toNativeFullPath(file);
    }

    std::wstring Path::fullPath() const
    {
        return fullPath_;
    }

    Path Path::absoluteDir() const
    {
        if (isFile())
        {
            std::wstring dirPath = fullPath_;
            size_t lastSepartor = dirPath.find_last_of(k_natvieDelimiter);
            dirPath.erase(lastSepartor);
            return dirPath;
        }
        else
            return fullPath_;
    }

    Path Path::parentDir() const
    {
        return std::filesystem::path(fullPath_).parent_path();
    }

    Path Path::rootPath() const
    {
        return std::filesystem::path(fullPath_).root_path();
    }

    std::wstring Path::fileName() const
    {
        return std::filesystem::path(fullPath_).filename();
        /*std::wstring filename;
        auto del = fullPath_.find_last_of(k_natvieDelimiter);
        if (del != fullPath_.npos)
            return fullPath_.substr(del + 1);
        else
            return fullPath_;*/
    }

    std::wstring Path::baseName() const
    {
        std::wstring filename = fileName();
        if (!filename.empty())
        {
            auto dot = filename.find_first_of('.');
            if (dot != filename.npos)
                return filename.substr(0, dot);
            else
                return filename;
        }
        else
            return std::wstring();
    }

    std::wstring Path::extension() const
    {
        return std::filesystem::path(fullPath_).extension();

      /*  auto dot = fullPath_.find_last_of('.');
        if (dot != fullPath_.npos)
            return fullPath_.substr(dot + 1);
    
        return std::wstring();*/
    }

    std::wstring Path::fullExtension() const
    {
        std::wstring filename = fileName();
        return filename.erase(0, baseName().size());
    }

    Path Path::operator/(const std::wstring& name) const
    {
        return std::filesystem::path(fullPath_).append(name).native();
        /*if (!fullPath_.empty())
            return fullPath_ + k_natvieDelimiter + name;
        else
            return fullPath_;*/
    }

    bool Path::isFile() const
    {
        return std::filesystem::is_regular_file(std::filesystem::path(fullPath_))
            || std::filesystem::is_symlink(std::filesystem::path(fullPath_));

        /*WIN32_FILE_ATTRIBUTE_DATA info;
        BOOL result = GetFileAttributesExW(fullPath_.c_str(), GetFileExInfoStandard, &info);
        return result && (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;*/
    }

    bool Path::isDir() const
    {
        return std::filesystem::is_directory(std::filesystem::path(fullPath_));

       /* WIN32_FILE_ATTRIBUTE_DATA info;
        BOOL result = GetFileAttributesExW(fullPath_.c_str(), GetFileExInfoStandard, &info);
        return result && (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;*/
    }

    std::wostream& operator<<(std::wostream& os, const Path& p)
    {
        return os << p.fullPath();
    }

}