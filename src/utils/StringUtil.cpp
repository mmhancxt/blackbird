#include "StringUtil.h"

namespace utils
{
    void Replace(std::string& str, const std::string& out, const std::string& in)
    {
        size_t index = str.find(out);
        if (index != std::string::npos)
        {
            auto len = out.size();
            if (index == 0)
            {
                str = in + str.substr(len);
            }
            else
            {
                str = str.substr(0, index) + in;
            }
        }
    }
}