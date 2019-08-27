#pragma once
namespace Storm
{
    namespace Utils
    {
        std::wstring fromLocal8Bit(const char* str, int size = -1);

        std::wstring fromLocal8Bit(const std::string& str);

        std::string toLocal8Bit(const wchar_t* wstr, int size = -1);

        std::string toLocal8Bit(const std::wstring& wstr);

        std::wstring fromUtf8(const char* str, int size = -1);

        std::wstring fromUtf8(const std::string& str);

        std::string toUtf8(const wchar_t* str, int size = -1);

        std::string toUtf8(const std::wstring& str);

        std::string toHex(unsigned int d);

        unsigned int fromHex(const std::string& h);

        std::string toHexCode(const std::string& d);

        std::string fromHexCode(const std::string& h);

        std::string simpleEncode(std::string text, char x);

        std::string simpleDecode(std::string text, char x);

        std::wstring appDataPath();

        void makeSureDirExist(const std::wstring& dir);

        std::wstring appProcessName();

        std::wstring applicationDirPath();

        void usleep(std::int64_t microsecs_to_sleep);

    };
}

namespace stdxx
{
    template<class K, class V>
    std::vector<K> keys(const std::map<K, V>& src)
    {
        std::vector<K> result;
        for (const auto& pair: src)
        {
            result.push_back(pair.first);
        }
        return result;
    }

    template<class K, class V>
    std::vector<V> values(const std::map<K, V>& src)
    {
        std::vector<V> result;
        for (const auto& pair : src)
        {
            result.push_back(pair.second);
        }
        return result;
    }

    template <class Str>
    std::vector<Str> split(const Str& src, const typename Str::value_type& sep, bool compress = true)
    {
        std::vector<Str> vec;
        size_t cur = 0;
        size_t next = 0;
        while (true)
        {
            next = src.find(sep, cur);
            if (next == Str::npos)
            {
                if (cur != Str::npos)
                {
                    vec.push_back(Str(src, cur, src.size() - cur));
                }
                break;
            }
            if (!compress || (next - cur) != 0)
            {
                vec.push_back(Str(src, cur, next - cur));
            }
            cur = next;
            if(next == (src.size()-1))
            {
                if(!compress)
                {
                    vec.push_back(Str(src, cur, 0));
                }
                break;
            }
            cur += 1;
        }

        return vec;
    }

    template <class Vec, class Str>
    typename Vec::value_type join(const Vec& vec, const Str& sep)
    {
        typename Vec::value_type res;
        auto it = vec.begin();
        if (it != vec.end())
        {
            res.insert(res.size(), *it);
            ++it;
        }
        while (it != vec.end())
        {
            res.insert(res.size(), sep);
            res.insert(res.size(), *it);
            ++it;
        }
        return res;
    }

    template <class Str>
    Str ltrim(const Str& src)
    {
        int first = 0;
        int last = int(src.size()) - 1;

        while (first <= last && std::isspace(src[first]))
            ++first;
        return Str(src, first, last - first + 1);
    }

    template <class Str>
    Str rtrim(const Str& src)
    {
        int last = int(src.size()) - 1;

        while (last >= 0 && std::isspace(src[last]))
            --last;
        return Str(src, 0, last + 1);
    }

    template <class Str>
    Str trim(const Str& src)
    {
        int first = 0;
        int last = int(src.size()) - 1;

        while (first <= last && std::isspace(src[first]))
            ++first;
        while (last >= first && std::isspace(src[last]))
            --last;
        return Str(src, first, last - first + 1);
    }

    template <class Str>
    Str to_upper(const Str& src)
    {
        Str res(src);
        std::transform(res.begin(), res.end(), res.begin(), Ascii::toUpper);
        return res;
    }

    template <class Str>
    Str to_lower(const Str& src)
    {
        Str res(src);
        std::transform(res.begin(), res.end(), res.begin(), Ascii::toLower);
        return res;
    }

    template <class Str>
    Str replace_substr(const Str& src, const Str& substr, const Str& replace)
    {
        Str str = src;
        size_t index = 0;
        while (true)
        {
            index = str.find(substr, index);
            if (index == Str::npos)
                break;

            str.replace(index, substr.size(), replace);

            index += replace.size();
        }

        return str;
    }

    inline bool ends_with(const std::wstring& src, const std::wstring& substr)
    {
        if (src.length() >= substr.length())
            return (0 == src.compare(src.length() - substr.length(), substr.length(), substr));
        return false;
    }

    inline bool ends_with(const std::string& src, const std::string& substr)
    {
        if (src.length() >= substr.length())
            return (0 == src.compare(src.length() - substr.length(), substr.length(), substr));
        return false;
    }

    inline bool begins_with(const std::string& src, const std::string& substr)
    {
        if (src.length() >= substr.length())
            return (0 == src.compare(0, substr.length(), substr));
        return false;
    }

    inline bool begins_with(const std::wstring& src, const std::wstring& substr)
    {
        if (src.length() >= substr.length())
            return (0 == src.compare(0, substr.length(), substr));
        return false;
    }

    template<class C, class V>
    auto contains_(const C& container, const V & value, int, int) -> decltype((container.find(value) != std::end(container)), bool())
    {
        return container.find(value) != std::end(container);
    }

    template<class C, class V>
    auto contains_(const C& container, const V & value, bool, int) -> decltype((container.contains(value)), bool())
    {
        return container.contains(value);
    }

    template<class C, class V>
    auto contains_(const C& container, const V & value, bool, bool)
    {
        return std::find(std::begin(container), std::end(container), value) != std::end(container);
    }

    template<class C, class V>
    auto contains(const C& container, const V & value)
    {
        return contains_(container, value, 0, 0);
    }

    template<class C>
    C repeat(const C& container, std::size_t num)
    {
        C result;
        while (num--)
        {
            std::copy(std::begin(container), std::end(container), std::back_inserter(result));
        }
        return result;
    }

    template<class C>
    auto sum(const C& conatiner)
    {
        const auto sz = conatiner.size();
        
        return sz ? std::accumulate(std::begin(conatiner), std::end(conatiner), typename C::value_type(), [](auto x, auto y) {return x + y; })
            : typename C::value_type();
    }

    template<class T, size_t sz>
    auto sum(const T(&conatiner)[sz])
    {
        return sz ? std::accumulate(conatiner, conatiner + sz, T(), [](auto x, auto y) {return x + y; })
            : T();
    }

    template<class C>
    auto avg(const C& conatiner)
    {
        const auto sz = conatiner.size();
        return sz ? std::accumulate(std::begin(conatiner), std::end(conatiner), typename C::value_type(), [](auto x, auto y) {return x + y; }) / sz
            : typename C::value_type();
    }

    template<class T, size_t sz>
    auto avg(const T(&conatiner)[sz])
    {
        return sz ? sum(conatiner) / sz
            : T();
    }

    template<class C>
    auto max(const C& conatiner)
    {
        return std::accumulate(std::begin(conatiner), std::end(conatiner), typename C::value_type(), [](auto x, auto y) {return (x > y) ? x : y; });
    }
    template<class C>
    auto min(const C& conatiner)
    {
        return std::accumulate(std::begin(conatiner), std::end(conatiner), typename C::value_type(), [](auto x, auto y) {return (x < y) ? x : y; });
    }

}

namespace Storm
{
    namespace Utils
    {
        using namespace stdxx;
    }
}
