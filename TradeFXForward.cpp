# include "TradeFXForward.h"
# include "PricerFXForward.h"

namespace minirisk {

ppricer_t TradeFXForward::pricer(const string& bccy) const
{
    return ppricer_t(new PricerFXForward(*this, bccy));
}

} // namespace minirisk

