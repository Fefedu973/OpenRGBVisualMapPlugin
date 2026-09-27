// SPDX-License-Identifier: GPL-2.0-or-later
#include "RoutingPerformance.h"
#include <stdexcept>
void TestRoutingPerformance()
{
    const auto check=[](bool value){if(!value)throw std::runtime_error("bounded routing performance report");};
    visual_performance::Routing p;
    auto* a=p.Add(reinterpret_cast<void*>(1),"slow\ncontroller");
    a->Add(30000000);a->Add(1000000);
    check(p.Find(reinterpret_cast<void*>(1))==a);
    check(p.Finish(32000000,100).empty());
    check(p.Finish(1000000,10099).empty());
    auto text=p.Finish(1000000,10100);
    check(text.find("frames=3")!=std::string::npos);
    check(text.find("slow controller SetColor calls=2 total_ms=31.000 avg_ms=15.500 max_ms=30.000")!=std::string::npos);
    check(text.find('\n')==std::string::npos);
    check(p.Finish(1000000,20100).empty()); // quick-only window stays silent
    for(uintptr_t i=1;i<=28;++i)check(p.Add(reinterpret_cast<void*>(i),"bounded")!=nullptr);
    check(p.Add(reinterpret_cast<void*>(29),"overflow")==nullptr);
    check(p.Finish(21000000,30100).find("controllers=28")!=std::string::npos);
    check(p.Finish(21000000,30101).empty());
}
