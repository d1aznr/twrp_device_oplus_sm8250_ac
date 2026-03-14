/*
 * Copyright (C) 2022-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <unordered_map>

using android::base::GetProperty;

struct ModelInfo {
    const char* brand;
    const char* model;
    const char* device;
    const char* twversion;
};

const std::unordered_map<int, ModelInfo> kModelInfoMap = {
    {20061,   {"OPPO",       "PEDM00",      "OP4E5D",       "OPPO_Find_X3"}                     }, // horee CN
    {20135,   {"OPPO",       "PDRM00",      "OP4EA7",       "OPPO_Reno5_Pro+"}                  }, // hima CN
    {20235,   {"OPPO",       "CPH2207",     "OP4F0BL1",     "OPPO_Find_X3_Neo"}                 }, // hima GLO
    {20161,   {"OPPO",       "PENM00",      "OP4EC1",       "OPPO_Reno6_Pro+"}                  }, // jin CN
    {20351,   {"OPPO",       "CPH2247",     "OP4F7FL1",     "OPPO_Reno6_Pro"}                   }, // jin EU
    {21615,   {"realme",     "RMX3366",     "RE546F",       "realme_GT_Master_Explorer_Edition"}}, // rivena CN
    {21619,   {"realme",     "RMX3370",     "RE5473",       "realme_GT_Neo_2"}                  }, // bitra CN
    {0x2169A, {"realme",     "RMX3370",     "RE879AL1",     "realme_GT_Neo_2"}                  }, // bitra IN
    {0,       {"OPLUS",      "SM8250-AC",   "SM8250-AC",    "SM8250-AC"}                        }, // Default
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void SetupModelProperties(const ModelInfo& info) {
    struct PropPair {
        const char* key;
        const char* value;
    } props[] = {
        {"ro.product.brand",            info.brand},
        {"ro.product.manufacturer",     info.brand},
        {"ro.product.model",            info.model},
        {"ro.product.name",             info.model},
        {"ro.product.device",           info.device},
        {"ro.commonsoft.ota",           info.device},
        {"ro.twrp.device_version",      info.twversion},
    };
    for (const auto& p : props) {
        OverrideProperty(p.key, p.value);
    }
}

int get_oplus_prjname() {
    std::string prj_str = GetProperty("ro.boot.prjname", "");
    for (char c : prj_str) {
        if (c >= 'A' && c <= 'F') {
            return std::strtol(prj_str.c_str(), nullptr, 16);
        }
    }
    return std::strtol(prj_str.c_str(), nullptr, 10);
}

void vendor_load_properties() {
    OverrideProperty("ro.separate.soft", GetProperty("ro.boot.prjname", "").c_str());
    auto prj_val = get_oplus_prjname();
    auto model_info = kModelInfoMap.find(prj_val);

    if (model_info == kModelInfoMap.end()) {
        switch(prj_val) {
            case 20137:
            case 20139:
                model_info = kModelInfoMap.find(20135);
                break;
            case 20163:
                model_info = kModelInfoMap.find(20161);
                break;
            case 20352:
                model_info = kModelInfoMap.find(20351);
                break;
            case 0x2161A:
                model_info = kModelInfoMap.find(21619);
                break;
            case 0x2169B:
                model_info = kModelInfoMap.find(0x2169A);
                break;
            // Handle unknown device models
            default:
                LOG(ERROR) << "Unknown prjname: " << prj_val << ", using default";
                model_info = kModelInfoMap.find(0);
        }  
    }

    SetupModelProperties(model_info->second);
}
