#include "Global.h"
#include "PortfolioUtils.h"
#include "TradePayment.h"
#include "TradeFXForward.h"

#include <numeric>

namespace minirisk {

void print_portfolio(const portfolio_t& portfolio)
{
    std::for_each(portfolio.begin(), portfolio.end(), [](auto& pt){ pt->print(std::cout); });
}

std::vector<ppricer_t> get_pricers(const portfolio_t& portfolio, const string& bccy)
{
    std::vector<ppricer_t> pricers(portfolio.size());
    std::transform( portfolio.begin(), portfolio.end(), pricers.begin()
                  , [bccy](auto &pt) -> ppricer_t { return pt->pricer(bccy); } );
    return pricers;
}

portfolio_values_t compute_prices(const std::vector<ppricer_t>& pricers, Market& mkt, const FixingDataServer& fds)
{//task: add error message
    portfolio_values_t prices(pricers.size());
    std::transform(pricers.begin(), pricers.end(), prices.begin()
        , [&mkt, &fds](auto &pp) -> std::pair<double, string> {
        try{
            return std::make_pair(pp->price(mkt, fds), "");
        }
        catch(const std::exception &e){
            return std::make_pair(std::numeric_limits<double>::quiet_NaN(), e.what());
        }
        
    });
    return prices;
}

std::pair<double, std::vector<std::pair<size_t, string>>> portfolio_total(const portfolio_values_t& values)
{
    std::pair<double, std::vector<std::pair<size_t, string>>> p_total;
    p_total.first = 0.0;
    for(size_t idx = 0; idx < values.size(); idx++){
        if(std::isnan(values[idx].first)){
            p_total.second.push_back(std::make_pair(idx, values[idx].second));
        }else{
            p_total.first += values[idx].first;
        }
    }
    return p_total;
}


std::vector<std::pair<string, portfolio_values_t>> compute_pv01_parallel(const std::vector<ppricer_t>& pricers, const Market& mkt,  const FixingDataServer& fds){
    std::vector<std::pair<string, portfolio_values_t>> pv01;  // PV01 per trade
    
    //string: like "IR.EDU" or "IR.USD"; Market::vec_risk_factor_t: related risk vectors
    std::map<std::string, Market::vec_risk_factor_t> pv01_risk_factors;

    const double bump_size = 0.01 / 100;

    // filter risk factors related to IR
    auto base = mkt.get_risk_factors(ir_rate_prefix + "([\\d]+[DWMY]\\.)?[A-Z]{3}");
    
    for(auto & tmp:base){
        auto name = tmp.first;
        pv01_risk_factors[ir_rate_prefix + name.substr(name.length() - 3, 3)].push_back(tmp);
    }

    // Make a local copy of the Market object, because we will modify it applying bumps
    // Note that the actual market objects are shared, as they are referred to via pointers
    Market tmpmkt(mkt);

    // compute prices for perturbated markets and aggregate results
    pv01.reserve(base.size());
    for (const auto& rf : pv01_risk_factors) {
        portfolio_values_t pv_up, pv_dn;
        std::vector<std::pair<string, double>> bumped(rf.second);
        if(rf.second.size() == 1){
            pv01.push_back(std::make_pair(rf.first, portfolio_values_t(pricers.size())));
        }else{
            pv01.push_back(std::make_pair("parallel "+ rf.first, portfolio_values_t(pricers.size())));
        }

        // bump down and price
        for(size_t i = 0; i < bumped.size(); i++){
            bumped[i].second = rf.second[i].second - bump_size;
        }
        tmpmkt.set_risk_factors(bumped);
        pv_dn = compute_prices(pricers, tmpmkt, fds);

        
        // bump up and price
        for(size_t i = 0; i < bumped.size(); i++){
            bumped[i].second = rf.second[i].second + bump_size;
        }
        tmpmkt.set_risk_factors(bumped);
        pv_up = compute_prices(pricers, tmpmkt, fds);


        // restore original market state for next iteration
        // (more efficient than creating a new copy of the market at every iteration)
        tmpmkt.set_risk_factors(rf.second);

        // compute estimator of the derivative via central finite differences
        double dr = 2.0 * bump_size;
        std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin()
            , [dr](std::pair<double,string> hi, std::pair<double,string> lo) -> std::pair<double,string> {
            if((!std::isnan(hi.first) & (!std::isnan(lo.first)))){
                return std::make_pair((hi.first - lo.first) / dr, "");
            }else{
                if(std::isnan(hi.first)){
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), hi.second);
                }else{
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), lo.second);
                }
            }
        });
    }

    return pv01;
}

std::vector<std::pair<string, portfolio_values_t>> compute_pv01_bucketed(const std::vector<ppricer_t>& pricers, const Market& mkt, const FixingDataServer& fds){
    std::vector<std::pair<string, portfolio_values_t>> pv01;  // PV01 per trade

    const double bump_size = 0.01 / 100;

    // filter risk factors related to IR
    auto base = mkt.get_risk_factors(ir_rate_prefix +"[\\d]+[DWMY]\\.[A-Z]{3}");

    // Make a local copy of the Market object, because we will modify it applying bumps
    // Note that the actual market objects are shared, as they are referred to via pointers
    Market tmpmkt(mkt);

    // compute prices for perturbated markets and aggregate results
    pv01.reserve(base.size());
    for (const auto& d : base) {
        portfolio_values_t pv_up, pv_dn;
        std::vector<std::pair<string, double>> bumped(1, d);
        pv01.push_back(std::make_pair("bucketed " + d.first, portfolio_values_t(pricers.size())));

        // bump down and price
        bumped[0].second = d.second - bump_size;
        tmpmkt.set_risk_factors(bumped);
        pv_dn = compute_prices(pricers, tmpmkt, fds);

        // bump up and price
        bumped[0].second = d.second + bump_size; // bump up
        tmpmkt.set_risk_factors(bumped);
        pv_up = compute_prices(pricers, tmpmkt, fds);


        // restore original market state for next iteration
        // (more efficient than creating a new copy of the market at every iteration)
        bumped[0].second = d.second;
        tmpmkt.set_risk_factors(bumped);

        // compute estimator of the derivative via central finite differences
        double dr = 2.0 * bump_size;
        std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin()
                       , [dr](std::pair<double,string> hi, std::pair<double,string> lo) -> std::pair<double,string> {
            if((!std::isnan(hi.first) & (!std::isnan(lo.first)))){
                return std::make_pair((hi.first - lo.first) / dr, "");
            }else{
                if(std::isnan(hi.first)){
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), hi.second);
                }else{
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), lo.second);
                }
            }
        });
    }

    return pv01;
}

std::vector<std::pair<string, portfolio_values_t>> compute_fx_delta(const std::vector<ppricer_t>& pricers, const Market& mkt,  const FixingDataServer& fds) {
    std::vector<std::pair<string, portfolio_values_t>> fx_delta;

    std::map<std::string, Market::vec_risk_factor_t> fx_delta_risk_factors;

    const double bump_size = 0.1 / 100;

    // filter risk factors related to FX.SPOT
    auto fx_spots = mkt.get_risk_factors(fx_spot_prefix + "[A-Z]{3}");

    
    // Make a local copy of the Market object, because we will modify it applying bumps
    // Note that the actual market objects are shared, as they are referred to via pointers
    Market tmpmkt(mkt);

    // compute prices for perturbated markets and aggregate results
    fx_delta.reserve(fx_spots.size());
    for (const auto& d : fx_spots) {
        portfolio_values_t pv_up, pv_dn;
        std::vector<std::pair<string, double>> bumped(1, d);
        fx_delta.push_back(std::make_pair(d.first, portfolio_values_t(pricers.size())));

        // bump down and price
        bumped[0].second = d.second * (1- bump_size);
        tmpmkt.set_risk_factors(bumped);
        pv_dn = compute_prices(pricers, tmpmkt, fds);

        // bump up and price
        bumped[0].second = d.second * (1 + bump_size); // bump up
        tmpmkt.set_risk_factors(bumped);
        pv_up = compute_prices(pricers, tmpmkt, fds);


        // restore original market state for next iteration
        // (more efficient than creating a new copy of the market at every iteration)
        bumped[0].second = d.second;
        tmpmkt.set_risk_factors(bumped);

        // compute estimator of the derivative via central finite differences
        double dr = 2.0 * bump_size * d.second;
        std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), fx_delta.back().second.begin()
                       , [dr](std::pair<double,string> hi, std::pair<double,string> lo) -> std::pair<double,string> {
            if((!std::isnan(hi.first) & (!std::isnan(lo.first)))){
                return std::make_pair((hi.first - lo.first) / dr, "");
            }else{
                if(std::isnan(hi.first)){
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), hi.second);
                }else{
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), lo.second);
                }
            }
        });
    }
    

    return fx_delta;
}


ptrade_t load_trade(my_ifstream& is)
{
    string name;
    ptrade_t p;

    // read trade identifier
    guid_t id;
    is >> id;

    if (id == TradePayment::m_id)
        p.reset(new TradePayment);
    else if(id == TradeFXForward::m_id){
        p.reset(new TradeFXForward);
    }
    else
        THROW("Unknown trade type:" << id);

    p->load(is);

    return p;
}

void save_portfolio(const string& filename, const std::vector<ptrade_t>& portfolio)
{
    // test saving to file
    my_ofstream of(filename);
    for( const auto& pt : portfolio) {
        pt->save(of);
        of.endl();
    }
    of.close();
}

std::vector<ptrade_t> load_portfolio(const string& filename)
{
    std::vector<ptrade_t> portfolio;

    // test reloading the portfolio
    my_ifstream is(filename);
    while (is.read_line())
        portfolio.push_back(load_trade(is));

    return portfolio;
}

void print_price_vector(const string& name, const portfolio_values_t& values)
{
    auto tmp = portfolio_total(values);
    
    std::cout
        << "========================\n"
        << name << ":\n"
        << "========================\n"
        << "Total: " << tmp.first
        <<"\n"
        << "Errors: " << tmp.second.size()
        << "\n========================\n";

    for (size_t i = 0, n = values.size(); i < n; ++i){
        if(std::isnan(values[i].first))
            std::cout << std::setw(5) << i << ": " << values[i].second << "\n";
        else
            std::cout << std::setw(5) << i << ": " << values[i].first << "\n";
    }

    std::cout << "========================\n\n";
}

} // namespace minirisk
