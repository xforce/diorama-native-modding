#include <locale>
#include <sstream>
#include <unordered_map>
#include <future>

namespace hooking
{
    void SetImageBase(uintptr_t);

    template <std::uint32_t FnvPrime, std::uint32_t OffsetBasis>
    struct basic_fnv_1
    {
        std::uint32_t operator()(std::string const& text) const
        {
            std::uint32_t hash = OffsetBasis;
            for (std::string::const_iterator it = text.begin(), end = text.end();
                it != end; ++it)
            {
                hash *= FnvPrime;
                hash ^= *it;
            }

            return hash;
        }
    };

    const std::uint32_t fnv_prime = (uint32_t)1099511u;
    const std::uint32_t fnv_offset_basis = (std::uint32_t)146959810u;

    typedef basic_fnv_1<fnv_prime, fnv_offset_basis> fnv_1;


    class PatternMatch
    {
    private:

        void *_address = nullptr;
        intptr_t _offset = 0;
    public:
        PatternMatch(void *address)
            : _address(address)
        {

        }

        PatternMatch& Adjust(intptr_t offset);

        intptr_t OriginalAddress();
        intptr_t Address();

        template<typename T>
        T* Cast()
        {
            return reinterpret_cast<T*>(static_cast<char*>(_address) + _offset);
        }

        template<>
        void* Cast()
        {
            return reinterpret_cast<void*>(static_cast<char*>(_address) + _offset);
        }
    };

    class Pattern
    {
    private:
        std::string _data;
        std::string _mask;
        std::string _pattern;
        uint64_t _hash;
        bool _matched = false;

        size_t _size = 0;
        std::vector<PatternMatch> _matches;
    public:
        Pattern(const std::string &_pattern);

        Pattern(const std::string &data, const std::string &mask)
            : _data(data),
            _mask(mask)
        { }

        bool DoMatch(uintptr_t offset);

        bool Search(bool onlyFirst);


        PatternMatch& Get(int32_t index);

        std::vector<PatternMatch> matches();
    };
}