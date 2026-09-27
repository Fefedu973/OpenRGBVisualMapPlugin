/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include "ZoneIdentity.h"
#include <algorithm>
#include <string>

namespace visual_persistence
{
inline bool ValidName(const std::string& name)
{
    return !name.empty() && name != "." && name != ".."
        && name.find_first_of("/\\:\0", 0, 4) == std::string::npos;
}

inline bool SameMember(const nlohmann::json& a, const nlohmann::json& b, bool moved_hid = false)
{
    if(!visual_identity::SameZone(a,b) || !a.contains("controller") || !b.contains("controller")) return false;
    for(const char* key : {"name","vendor","serial","location"})
    {
        if(!a["controller"].contains(key) || !b["controller"].contains(key)) return false;
        if(moved_hid && std::string(key)=="location"
           && a["controller"][key].is_string() && b["controller"][key].is_string()
           && a["controller"][key].get<std::string>().find("HID: ")==0
           && b["controller"][key].get<std::string>().find("HID: ")==0) continue;
        if(a["controller"][key] != b["controller"][key]) return false;
    }
    return true;
}

// Replace live members while retaining descriptors for unplugged devices.
// Explicit removal is handled by the editor before this merge.
inline nlohmann::json Merge(const nlohmann::json& saved, const nlohmann::json& live,
                            const nlohmann::json& grid)
{
    auto result = saved.is_object() ? saved : nlohmann::json::object();
    auto members = result.value("ctrl_zones", nlohmann::json::array());
    for(const auto& entry : live)
    {
        auto found = std::find_if(members.begin(), members.end(), [&](const auto& old) { return SameMember(old,entry); });
        if(found == members.end())
        {
            auto matches = [&](const auto& old) { return SameMember(old,entry,true); };
            if(std::count_if(members.begin(),members.end(),matches)==1)
                found=std::find_if(members.begin(),members.end(),matches);
        }
        if(found == members.end()) members.push_back(entry); else found->update(entry, true);
    }
    result["ctrl_zones"] = std::move(members);
    result["grid_settings"] = grid;
    return result;
}
}
