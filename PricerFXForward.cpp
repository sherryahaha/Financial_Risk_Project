# include "PricerFXForward.h"
# include "TradeFXForward.h"
# include "CurveDiscount.h"
# include "CurveFXSpot.h"
# include "CurveFXForward.h"

namespace minirisk
{
PricerFXForward::PricerFXForward(const TradeFXForward& trf, const string& bccy)
    : Pricer(bccy)
    , m_amt(trf.quantity())
    , m_fd(trf.fdate())
    , m_sd(trf.sdate())
    , m_s(trf.strike())
    , m_ccy1(trf.ccy1())
    , m_ccy2(trf.ccy2())
{
}
double PricerFXForward::price(Market& mkt, const FixingDataServer& fds) const{
    //std::cout << m_amt << m_fd << m_sd << m_s << m_ccy1 << m_ccy2 << std::endl;
    
    ptr_disc_curve_t disc = mkt.get_discount_curve(ir_curve_discount_name(m_ccy2));
    double b = disc->df(m_sd);

    ptr_spot_curve_t spot_curves_base = mkt.get_spot_curve(m_ccy2 + "." + m_bccy);
    double spot_price_base = spot_curves_base->spot();

    //std::cout << b << " " << spot_price_base << std::endl;
    
    double spot_price = 0.0;

    if (mkt.today() > m_fd) {
        spot_price = fds.get(fx_spot_name(m_ccy1, m_ccy2), m_fd);
    }
    else {
        if (mkt.today() == m_fd) {
            auto fixing_t1 = fds.lookup(fx_spot_name(m_ccy1, m_ccy2), m_fd);
            if (fixing_t1.second) {
                spot_price = fixing_t1.first;
            }
            else {
                ptr_spot_curve_t spot_curves = mkt.get_spot_curve(m_ccy1 + "." + m_ccy2);
                spot_price = spot_curves->spot();
            }
        }
        else {
            ptr_forward_curve_t fp_curves = mkt.get_forward_curve(fx_fw_name(m_ccy1, m_ccy2));
            spot_price = fp_curves->forward_price(m_fd);
        }
    }
       
    
    double payoff = m_amt * (spot_price - m_s);

    double base_price = b * spot_price_base;
    
    return payoff * base_price ;
}
}
