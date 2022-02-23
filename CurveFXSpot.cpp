#include "CurveFXSpot.h"
#include "Market.h"

namespace minirisk {


CurveFXSpot::CurveFXSpot(Market *mkt, const Date& today, const string& spot_name)
    : m_today(today)
    , m_name(spot_name)
    , m_spot(mkt->get_fx_spot(spot_name.substr(spot_name.length() - 7, 3), spot_name.substr(spot_name.length() - 3, 3)))
{
}

double CurveFXSpot::spot() const
{
    return m_spot;
}

} // namespace minirisk
