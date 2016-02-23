#include "hooking_patterns.h"

#include <Windows.h>
#include <winternl.h>
using namespace hooking;


class PatternCache
{
private:
    std::unordered_map<std::string, std::vector<PatternMatch>> _cache;

public:
    void Add(const std::string &pattern, const std::vector<PatternMatch> &matches);
    bool Contains(const std::string &pattern);
    std::vector<PatternMatch> & Get(const std::string &pattern);
};

class SectionInfo
{
private:
    uintptr_t _begin;
    uintptr_t _end;
public:

    decltype(auto) begin() { return _begin; }
    decltype(auto) end() { return _end; }

    SectionInfo(uintptr_t begin, uintptr_t end)
        : _begin(begin),
        _end(end)
    {

    }
};

class ExecutableInfo
{
private:
    uintptr_t _begin;
    uintptr_t _end;
    uintptr_t _imageSize;
    uintptr_t _executableAddress;
    bool _ssePatternSearching = false;
    std::wstring _workingPath;

    std::vector<SectionInfo> sections;

public:
    ExecutableInfo()
        : _begin(0), _end(0)
    {

    }

    void SetExecutableAddress(uintptr_t addr)
    {
        _executableAddress = addr;
    }

    uintptr_t GetExecutableAddress()
    {
        return _executableAddress;
    }

    void SetWorkingPath(const std::wstring& str)
    {
        _workingPath = str;
    }

    void EnsureInit(uintptr_t executableAddress = 0)
    {
        if ((_begin && executableAddress == _executableAddress) || executableAddress == 0)
        {
            return;
        }

        _executableAddress = executableAddress;
        _begin = _executableAddress;

        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(_executableAddress);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        {
            throw std::runtime_error("Invalid DOS Signature");
        }

        PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)((_executableAddress + (dosHeader->e_lfanew * sizeof(char))));
        if (header->Signature != IMAGE_NT_SIGNATURE)
        {
            throw std::runtime_error("Invalid NT Signature");
        }

        _end = _begin + header->OptionalHeader.BaseOfCode + header->OptionalHeader.SizeOfCode;
        _imageSize = header->OptionalHeader.SizeOfImage;

        PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(header);

        for (int32_t i = 0; i < header->FileHeader.NumberOfSections; i++, section++)
        {
            bool executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
            bool readable = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
            //bool writeable = (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

            if (readable && executable)
            {
                auto beg = (header->OptionalHeader.ImageBase + section->VirtualAddress);

                uint32_t sizeOfData = std::min(section->SizeOfRawData, section->Misc.VirtualSize);

                sections.emplace_back(beg, beg + sizeOfData);
            }
        }
    }

    decltype(auto) begin() { return sections.begin(); }
    decltype(auto) end() { return sections.end(); }

    uintptr_t imageSize() {
        return _imageSize;
    };
};

static ExecutableInfo executableInfo;
static PatternCache cache;

PatternMatch& PatternMatch::Adjust(intptr_t offset)
{
    _offset = offset;
    return *this;
}

intptr_t PatternMatch::OriginalAddress()
{
    return reinterpret_cast<intptr_t>(_address);
}

intptr_t PatternMatch::Address()
{
    //return reinterpret_cast<uintptr_t>(_address);
    return reinterpret_cast<intptr_t>(static_cast<char*>(_address) + _offset);
}




void PatternCache::Add(const std::string &pattern, const std::vector<PatternMatch> &matches)
{
    _cache[pattern] = matches;
}

bool PatternCache::Contains(const std::string &pattern)
{
    return (_cache.find(pattern) != _cache.end());
}

std::vector<PatternMatch> & PatternCache::Get(const std::string &pattern)
{
    if (!Contains(pattern))
    {
        return _cache[pattern];
    }
    else
    {
        return _cache[pattern];
    }
}

static void GenerateMaskAndData(const std::string &pattern, std::string &mask, std::string &data)
{
    const static std::locale loc;

    std::stringstream dataStream;
    std::stringstream maskStream;

    for (auto ch = pattern.begin(); ch != pattern.end(); ++ch)
    {
        if (*ch == '?')
        {
            dataStream << '\x00';
            maskStream << '?';
        }
        else if (std::isalnum(*ch, loc))
        {
            auto ch1 = *ch;
            auto ch2 = *(++ch);
            char str[] = { ch1,  ch2 };
            char digit = static_cast<char>(strtol(str, nullptr, 16));
            dataStream << digit;
            maskStream << 'x';
        }
    }

    data = dataStream.str();
    mask = maskStream.str();
}

Pattern::Pattern(const std::string &_pattern) : _pattern(_pattern)
{
    if (_pattern.empty())
    {
        throw std::exception("No pattern given");
    }

    GenerateMaskAndData(_pattern, _mask, _data);

    _hash = fnv_1()(_pattern);
}

bool Pattern::DoMatch(uintptr_t offset)
{
    const char *pattern_ = _data.c_str();
    const char *mask = _mask.c_str();

    char *ptr = reinterpret_cast<char*>(offset);

    for (size_t i = 0; i < _mask.size(); i++)
    {
        if (mask[i] == '?')
        {
            continue;
        }

        if (_data.length() < i || pattern_[i] != ptr[i])
        {
            return false;
        }
    }

    _matches.push_back(PatternMatch(ptr));

    return true;
}

bool Pattern::Search(bool)
{
    std::vector<std::future<std::vector<PatternMatch>>> futureHandles = {};

    {
        for (auto& section : executableInfo)
        {
            auto secSize = section.end() - section.begin();
            if (secSize > 1)
            {
                auto partSize = secSize;
                auto rest = secSize % partSize;
                for (uintptr_t i = section.begin(); i < section.end() - rest; i += partSize)
                {
                    auto handle = std::async(std::launch::async, [&](uintptr_t start, uintptr_t end) -> std::vector<PatternMatch> {
                        std::vector<PatternMatch> vecMatches;
                        for (uintptr_t offset = start; offset < end; ++offset)
                        {
                            if (DoMatch(offset))
                            {
                                vecMatches.push_back(PatternMatch(reinterpret_cast<char*>(offset)));
                            }
                        }
                        return vecMatches;
                    }, i, i + partSize);

                    futureHandles.push_back(std::move(handle));
                }
            }
        }
    }

    _matches.clear();

    for (auto &handle : futureHandles)
    {
        auto matches = handle.get();
        _matches.insert(_matches.end(), matches.begin(), matches.end());
    }

    return true;
}

PatternMatch& Pattern::Get(int32_t index)
{
    
    if (cache.Contains(_pattern))
    {
        return cache.Get(_pattern)[index];
    }
    else
    {
        if (!_matched)
        {
            Search(index == 0);
        }

        cache.Add(_pattern, _matches);

        if (matches().size() == 0)
        {
            throw std::runtime_error("Could not find pattern!");
        }

        return cache.Get(_pattern)[index];
    }
}

std::vector<PatternMatch> Pattern::matches()
{
    if (cache.Contains(_pattern))
    {
        return cache.Get(_pattern);
    }
    else
    {
        if (!_matched)
        {
            Search(false);
        }

        cache.Add(_pattern, _matches);
        return cache.Get(_pattern);
    }
}













void hooking::SetImageBase(uintptr_t address)
{
    executableInfo.EnsureInit(address);
}
