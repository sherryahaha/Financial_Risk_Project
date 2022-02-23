#include "PricerPayment.h"
#include "TradePayment.h"
#include "CurveDiscount.h"

namespace minirisk {

PricerPayment::PricerPayment(const TradePayment& trd, const string& bccy):
    Pricer(bccy)
    , m_amt(trd.quantity())
    , m_dt(trd.delivery_date())
    , m_ir_curve(ir_curve_discount_name(trd.ccy()))
    , m_fx_ccy(trd.ccy() == m_bccy ? "" : fx_spot_name(trd.ccy(),m_bccy))
{
}


double PricerPayment::price(Market& mkt, const FixingDataServer& fds) const
{
    ptr_disc_curve_t disc = mkt.get_discount_curve(m_ir_curve);
    double df = disc->df(m_dt); // this throws an exception if m_dt<today

    // This PV is expressed in m_ccy. It must be converted in m_fx_ccy.
    if (m_fx_ccy != ""){
        ptr_spot_curve_t spot_curves = mkt.get_spot_curve(m_fx_ccy);
        double spot_price = spot_curves->spot();
        df *= spot_price;
    }

    return m_amt * df;
}

} // namespace minirisk


