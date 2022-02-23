#pragma once
#include <map>
#include "Date.h"
#include "Global.h"
#include "Streamer.h"

namespace minirisk
{

    struct FixingDataServer
    {
    public:
        FixingDataServer(const std::string& filename)
        {
            std::ifstream is(filename);
            std::string name;
            std::string date_str;
            double value;
            do {
                is >> name >> date_str >> value;

                unsigned y = std::atoi(date_str.substr(0, 4).c_str());
                unsigned m = std::atoi(date_str.substr(4, 2).c_str());
                unsigned d = std::atoi(date_str.substr(6, 2).c_str());
                Date m_date(y, m, d);

                m_fixings.emplace(std::make_pair(name, m_date), value);
            } while (is);
        }
        double get(const string& name, const Date& t) const;
        // return the fixing if available, otherwise trigger an error
        std::pair < double, bool > lookup(const string& name, const Date& t) const;
        // return the fixing if available, NaN otherwise, and set the flag if found

    private:
        std::map<std::pair<string, Date>, double> m_fixings;
    };

}
