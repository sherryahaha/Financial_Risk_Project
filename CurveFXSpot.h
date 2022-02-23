#pragma once

#include <string>

#include "ICurve.h"
#include "Market.h"
#include "MarketDataServer.h"

namespace minirisk {

struct CurveFXSpot : ICurveFXSpot
{
private:
    Date m_today;
    std::string m_name;
    double m_spot;
    
public:
    CurveFXSpot(Market *mkt, const Date& today, const std::string& spot_name);
    
    virtual Date today() const { return m_today; }
    
    virtual std::string name() const { return m_name; }

    //return fx spot price
    double spot() const;

 
};

} // namespace minirisk
