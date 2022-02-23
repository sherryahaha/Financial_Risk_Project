#include "TradePayment.h"
#include "PricerPayment.h"

namespace minirisk {

ppricer_t TradePayment::pricer(const string& bccy) const
{
    return ppricer_t(new PricerPayment(*this, bccy));
}

} // namespace minirisk
