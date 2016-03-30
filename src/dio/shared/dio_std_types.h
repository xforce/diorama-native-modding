#include "allocator.h"

#include <string>

namespace dio
{
    using string = std::basic_string<char, std::char_traits<char>, mallocator<char>>;
}