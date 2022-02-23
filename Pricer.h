#pragma once
#include "IPricer.h"
namespace minirisk{
struct Pricer : IPricer
{
protected:
    Pricer(const string& bccy) : m_bccy(bccy) {}
    string m_bccy;
};

}
