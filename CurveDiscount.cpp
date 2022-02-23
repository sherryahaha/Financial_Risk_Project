#include "CurveDiscount.h"
#include "Market.h"
#include "Streamer.h"

#include <cmath>


namespace minirisk {

CurveDiscount::CurveDiscount(Market *mkt, const Date& today, const string& curve_name)
    : m_today(today)
    , m_name(curve_name)
    , m_rate(mkt->get_yield(curve_name.substr(ir_curve_discount_prefix.length(),3)))
//task:modify m_rate
{
}

double  CurveDiscount::df(const Date& t) const
{//task: modify df
    MYASSERT((!(t < m_today)), "Curve " << m_name << ", DF not available before anchor date "<< m_today <<", requested " << t);
    double dt = time_frac(m_today, t);
    // format like "IR.1W.USD"
    if(m_rate.at(0) == 0){
        MYASSERT(!((t - m_today)/365.0 > m_rate.rbegin()->first) , "Curve " << m_name << ", DF not available beyond last tenor date "<< Date(m_today.get_serial() + (unsigned int)(m_rate.rbegin()->first*365)) <<", requested " << t);
        auto it = m_rate.lower_bound(dt);
        // Ti == dt
        if(it->first == dt){
            return std::exp(-it->second);
        }else{
            auto rt_iplus1 = it->second;
            auto t_iplus1 = it->first;
            auto rt_i = (--it)->second;
            auto t_i = it->first;
            auto ri_iplus1 = (rt_iplus1 - rt_i) / (t_iplus1 - t_i);
            return std::exp(-rt_i - ri_iplus1 * (dt - t_i));
        }
    // format like "IR.USD"
    }else{
        return std::exp(-m_rate.at(0) * dt);
    }
}

} // namespace minirisk
