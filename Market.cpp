#include "Market.h"
#include "CurveDiscount.h"
#include "CurveFXSpot.h"
#include "CurveFXForward.h"

#include <vector>

namespace minirisk {

template <typename I, typename T>
std::shared_ptr<const I> Market::get_curve(const string& name)
{
    ptr_curve_t& curve_ptr = m_curves[name];
    int temp = 0;
    if (!curve_ptr.get()){
        curve_ptr.reset(new T(this, m_today, name));
        temp = 1;
    }
    std::shared_ptr<const I> res = std::dynamic_pointer_cast<const I>(curve_ptr);
    MYASSERT(res, "Cannot cast object with name " << name << " to type " << typeid(I).name());
    return res;
}

const ptr_disc_curve_t Market::get_discount_curve(const string& name)
{
    return get_curve<ICurveDiscount, CurveDiscount>(name);
}

const ptr_spot_curve_t Market::get_spot_curve(const string& name)
{
    return get_curve<ICurveFXSpot, CurveFXSpot>(name);
}

const ptr_forward_curve_t Market::get_forward_curve(const string& name)
{
    return get_curve<ICurveFXForward, CurveFXForward>(name);
}

double Market::from_mds(const string& objtype, const string& name)
{
    auto ins = m_risk_factors.emplace(name, std::numeric_limits<double>::quiet_NaN());
    if (ins.second) { // just inserted, need to be populated
        MYASSERT(m_mds, "Cannot fetch " << objtype << " " << name << " because the market data server has been disconnnected");
        ins.first->second = m_mds->get(name);
    }
    return ins.first->second;
}

unsigned Market::transform_to_years(const string& time){
    auto t_scale = time.back();
    unsigned day_basic = 0;
    switch (t_scale){
        case 'D':
            day_basic = 1;
            break;
        case 'W':
            day_basic = 7;
            break;
        case 'M':
            day_basic = 30;
            break;
        case 'Y':
            day_basic = 365;
            break;
        default:
            //day_basic = 0;
            MYASSERT(t_scale, "not in the regular time range: " << t_scale; )
    }
    unsigned time_duration = std::stoi(time.substr(0, time.length() - 1));
    return time_duration * day_basic;
}


std::map<double, double> Market::get_yield(const string& ccyname)
{
    string pattern(ir_rate_prefix + "([\\d]+[DWMY])\\." + ccyname);
    std::vector<std::string> match_data;
    std::map<double, double> rates;
    if(m_mds){
        match_data = m_mds->match(pattern);
    }else{
        std::regex r(pattern);
        std::smatch result;
        for(const auto &rf:m_risk_factors){
            if(std::regex_match(rf.first, result, r)){
                match_data.push_back(result[1]);
            }
        }
    }
    if(match_data.size()>0){
        rates[0.0] = 0.0;
        for(auto const &it:match_data){
            double years = transform_to_years(it) / 365.0;
            rates[years] =  years * from_mds("yield curve", ir_rate_prefix +it+"."+ccyname);
        }
        return rates;
    }
    else{
        string name(ir_rate_prefix + ccyname);
        rates[0.0] = from_mds("yield curve", name);
        return rates;
    }
};

const double Market::get_fx_spot(const string& ccy1, const string& ccy2)
{
    double c1 = ccy1 == "USD" ? 1.0 : from_mds("fx spot", fx_spot_prefix + ccy1);
    double c2 = ccy2 == "USD" ? 1.0 : from_mds("fx spot", fx_spot_prefix + ccy2);
    return c1 / c2;
}

void Market::set_risk_factors(const vec_risk_factor_t& risk_factors)
{
    clear();
    for (const auto& d : risk_factors) {
        auto i = m_risk_factors.find(d.first);
        MYASSERT((i != m_risk_factors.end()), "Risk factor not found " << d.first);
        i->second = d.second;
    }
}

Market::vec_risk_factor_t Market::get_risk_factors(const std::string& expr) const
{
    vec_risk_factor_t result;
    std::regex r(expr);
    for (const auto& d : m_risk_factors)
        if (std::regex_match(d.first, r))
            result.push_back(d);
    return result;
}

} // namespace minirisk
