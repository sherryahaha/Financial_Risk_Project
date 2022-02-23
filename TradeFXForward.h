#pragma once

#include "Trade.h"

namespace minirisk {

struct TradeFXForward : Trade<TradeFXForward>
{
    friend struct Trade<TradeFXForward>;

    static const guid_t m_id;
    static const std::string m_name;

    virtual ppricer_t pricer(const string& bccy) const;


    const string& ccy1() const
      {
          return m_ccy1;
      }

    const string& ccy2() const
      {
          return m_ccy2;
      }

    const Date& fdate() const
      {
          return m_fdate;
      }

    const Date& sdate() const
      {
          return m_sdate;
      }

    const double strike() const
      {
          return m_strike;
      }

private:
    void save_details(my_ofstream& os) const
    {
        os << m_ccy1 << m_ccy2 << m_strike << m_fdate << m_sdate;
    }

    void load_details(my_ifstream& is)
    {
        is >> m_ccy1 >> m_ccy2 >> m_strike >> m_fdate >> m_sdate;
    }
    
    void print_details(std::ostream& os) const
    {
        os << format_label("Stike level") << m_strike << std::endl;
        os << format_label("Base Currency") << m_ccy1 << std::endl;
        os << format_label("Quote Currency") << m_ccy2 << std::endl;
        os << format_label("Fixing Date") << m_fdate << std::endl;
        os << format_label("Settlement Date") << m_sdate << std::endl;
    }
    
    string m_ccy1;
    string m_ccy2;
    double m_strike;
    Date m_fdate;
    Date m_sdate;
};

} // namespace minirisk
