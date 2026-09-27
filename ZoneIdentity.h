/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <limits>
#include <nlohmann/json.hpp>

namespace visual_identity
{
struct Zone
{
    unsigned index = 0;
    bool segment = false;
    unsigned segment_index = 0;
    bool operator==(const Zone& other) const
    {
        return index == other.index && segment == other.segment
            && (!segment || segment_index == other.segment_index);
    }
};
inline bool ReadIndex(const nlohmann::json& entry, const char* name, unsigned& out)
{
    if(!entry.contains(name) || !entry[name].is_number_integer()) return false;
    const auto& value = entry[name];
    if(value < 0 || value > std::numeric_limits<unsigned>::max()) return false;
    out = value.get<unsigned>();
    return true;
}
inline bool Read(const nlohmann::json& entry, Zone& out)
{
    out = {};
    if(!entry.is_object() || !ReadIndex(entry,"zone_idx",out.index)) return false;
    if(entry.contains("is_segment"))
    {
        if(!entry["is_segment"].is_boolean()) return false;
        out.segment = entry["is_segment"].get<bool>();
    }
    // Legacy entries denote whole zones. A segment must identify its index;
    // silently selecting segment0 would merge unrelated physical components.
    return !out.segment || ReadIndex(entry,"segment_idx",out.segment_index);
}
inline bool Matches(const nlohmann::json& entry, unsigned zone, bool is_segment, unsigned segment)
{
    Zone saved;
    return Read(entry,saved) && saved == Zone{zone,is_segment,segment};
}
inline bool SameZone(const nlohmann::json& left, const nlohmann::json& right)
{
    Zone a,b;
    return Read(left,a) && Read(right,b) && a == b;
}
}
