#pragma once

#include "Macros.h"
#include <string>
#include <array>

namespace minirisk {

struct Date
{
public:
    static const unsigned first_year = 1900;
    static const unsigned last_year = 2200;
    static const unsigned n_years = last_year - first_year;

private:
    static std::string padding_dates(unsigned);

    // number of days elapsed from beginning of the year
    unsigned day_of_year(unsigned y, unsigned m, unsigned d) const;

    friend long operator-(const Date& d1, const Date& d2);

    static const std::array<unsigned, 12> days_in_month;  // num of days in month M in a normal year
    static const std::array<unsigned, 12> days_ytd;      // num of days since 1-jan to 1-M in a normal year
    static const std::array<unsigned, n_years> days_epoch;   // num of days since 1-jan-1900 to 1-jan-yyyy (until 2200)

public:
    // Default constructor
    Date() : m_serial(0) {}

    // Constructor where the input value is checked.
    Date(unsigned year, unsigned month, unsigned day)
    {
        init(year, month, day);
    }
    
    void init(unsigned year, unsigned month, unsigned day)
    {
        check_valid(year, month, day);
        m_serial = serial(year, month, day);
    }

    // Constructor using serial
    Date(unsigned serial)
    {
        init(serial);
    }

    void init(unsigned serial)
    {
        check_valid(serial);
        m_serial = serial;
    }

    static void check_valid(unsigned y, unsigned m, unsigned d);
    static void check_valid(unsigned s);

    bool operator<(const Date& d) const
    {
        return m_serial < d.m_serial;
    }

    bool operator==(const Date& d) const
    {
        return m_serial == d.m_serial;
    }

    bool operator>(const Date& d) const
    {
        return d < (*this);
    }

    // number of days since 1-Jan-1900
    unsigned serial(unsigned y, unsigned m, unsigned d) const
    {
        return days_epoch[y - 1900] + day_of_year(y, m, d);
    }

    unsigned get_serial() const { return m_serial; }

    void to_y_m_d(unsigned* y, unsigned* m, unsigned* d) const;

    static bool is_leap_year(unsigned yr);

    // In YYYYMMDD format
    std::string to_string(bool pretty = true) const
    {
        unsigned m_y, m_m, m_d;
        to_y_m_d(&m_y, &m_m, &m_d);
        return pretty
            ? std::to_string((int)m_d) + "-" + std::to_string((int)m_m) + "-" + std::to_string(m_y)
            : std::to_string(m_serial);
    }
    
private:
    unsigned m_serial;
};

long operator-(const Date& d1, const Date& d2);

inline double time_frac(const Date& d1, const Date& d2)
{
    return static_cast<double>(d2 - d1) / 365.0;
}

} // namespace minirisk
