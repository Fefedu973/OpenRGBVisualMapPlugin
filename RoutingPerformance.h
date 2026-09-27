// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

// Diagnostic metadata only, owned by the image-routing worker. Bounded storage
// and one report per ten seconds, and only when a route exceeded twenty ms.
namespace visual_performance
{
class Routing
{
public:
    struct Counter
    {
        const void* identity = nullptr;
        std::string name;
        uint64_t calls = 0, total_ns = 0, max_ns = 0;
        void Add(uint64_t ns) { ++calls; total_ns += ns; max_ns = std::max(max_ns,ns); }
    };
    Counter* Find(const void* identity)
    {
        for(std::size_t i=0;i<count;++i) if(counters[i].identity==identity) return &counters[i];
        return nullptr;
    }
    Counter* Add(const void* identity, std::string name)
    {
        if(auto* found=Find(identity)) return found;
        if(count==counters.size()) return nullptr;
        name.resize(std::min<std::size_t>(name.size(),96));
        for(char& c:name) if(static_cast<unsigned char>(c)<32 || c==';' || c=='|') c=' ';
        auto& result=counters[count++];result=Counter{};result.identity=identity;result.name=std::move(name);
        return &result;
    }
    std::string Finish(uint64_t frame_ns, uint64_t now_ms)
    {
        if(!started) { started=true; start_ms=now_ms; }
        ++frames; total_ns+=frame_ns; max_ns=std::max(max_ns,frame_ns);
        if(now_ms-start_ms<10000) return {};
        std::string report;
        if(max_ns>20000000)
        {
            std::ostringstream out;out<<std::fixed<<std::setprecision(3);
            out<<"[VisualMap Perf] RouteImage frames="<<frames
               <<" avg_ms="<<(total_ns/1e6/frames)<<" max_ms="<<(max_ns/1e6)
               <<" window_ms="<<(now_ms-start_ms)<<" controllers="<<count;
            for(std::size_t i=0;i<count;++i)
            {
                const auto& c=counters[i];
                out<<" | "<<c.name<<" SetColor calls="<<c.calls
                   <<" total_ms="<<(c.total_ns/1e6)
                   <<" avg_ms="<<(c.calls?c.total_ns/1e6/c.calls:0)
                   <<" max_ms="<<(c.max_ns/1e6);
            }
            report=out.str();
        }
        count=0;frames=0;total_ns=0;max_ns=0;start_ms=now_ms;
        return report;
    }
private:
    std::array<Counter,28> counters{};
    std::size_t count=0;
    bool started=false;
    uint64_t start_ms=0,frames=0,total_ns=0,max_ns=0;
};
}
