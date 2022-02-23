#include "CurveFXForward.h"
#include "Market.h"

namespace minirisk {
CurveFXForward::CurveFXForward(Market *mkt, const Date& today, const string& spot_name)
    : m_today(today)
    , m_name(spot_name)
    , m_st0(mkt->get_spot_curve(fx_spot_prefix + spot_name.substr(fx_fw_prefix.length(), 7))->spot())
    , m_df_curve1(mkt->get_discount_curve(ir_curve_discount_name(spot_name.substr(spot_name.length()-7, 3))))
    , m_df_curve2(mkt->get_discount_curve(ir_curve_discount_name(spot_name.substr(spot_name.length()-3, 3))))

{

}


double CurveFXForward::forward_price (const Date & t) const{
    double b1 = m_df_curve1->df(t);
    double b2 = m_df_curve2->df(t);
    return m_st0*b1/b2;
};


}
