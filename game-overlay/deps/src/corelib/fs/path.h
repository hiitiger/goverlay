#pragma once
namespace Storm
{
    class Path
    {
        std::wstring fullPath_;

    public:
        Path();
        Path(const std::wstring& file);

        std::wstring fullPath() const;

        Path absoluteDir() const;
        Path parentDir() const;

        Path rootPath() const;

        std::wstring fileName() const;
        std::wstring baseName() const;
        std::wstring extension() const;
        std::wstring fullExtension() const;

        Path operator / (const std::wstring& name) const;

        bool isFile() const;
        bool isDir() const;
    };

    std::wostream& operator << (std::wostream& os, const Path&);

}