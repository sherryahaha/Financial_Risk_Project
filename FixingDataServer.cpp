#include "FixingDataServer.h"
#include <limits>
#include <string>

namespace minirisk {

    double FixingDataServer::get(const string& name, const Date& t) const
    {
        auto file_name = m_fixings.find(std::make_pair(name, t));
        MYASSERT(file_name != m_fixings.end(), "Fixing not found: " << name << ", " << t);
        return file_name->second;
    }

    std::pair<double, bool> FixingDataServer::lookup(const string& name, const Date& t) const
    {
        auto file_name = m_fixings.find(std::make_pair(name, t));
        if (file_name == m_fixings.end())
            return (std::make_pair(std::numeric_limits<double>::quiet_NaN(), false));
        else
        {
            return std::make_pair(file_name->second, true);
        }
    }

}


