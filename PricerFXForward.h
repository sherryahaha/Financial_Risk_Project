#include "TradeFXForward.h"
#include "Pricer.h"

namespace minirisk {

struct PricerFXForward : Pricer
{
    PricerFXForward(const TradeFXForward& trf, const string& bccy);
    virtual double price(Market& mkt, const FixingDataServer& fds) const;

private:
    double m_amt;
    Date m_fd; //fixing date
    Date m_sd; //settle date
    double m_s; //strike
    string m_ccy1;
    string m_ccy2;
};

} // namespace minirisk

