
#pragma once

#include <string>

#include "ICurve.h"
#include "Market.h"
#include "MarketDataServer.h"

namespace minirisk {

struct CurveFXForward : ICurveFXForward
{
private:
    Date m_today;
    std::string m_name;
    double m_st0;
    ptr_disc_curve_t m_df_curve1;
    ptr_disc_curve_t m_df_curve2;
    
public:
    CurveFXForward(Market *mkt, const Date& today, const std::string& spot_name);
    
    virtual Date today() const { return m_today; }
    
    virtual std::string name() const { return m_name; }

    virtual double forward_price(const Date & t) const;

 
};

} // namespace minirisk



