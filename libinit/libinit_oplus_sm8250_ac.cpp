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
    const char* brand;              // ro.product.brand
    const char* device;             // ro.product.device
    const char* manufacturer;       // ro.product.manufacturer
    const char* model;              // ro.product.model
    const char* base_name;          // ro.product.name
    const char* commonsoft;         // ro.commonsoft.ota
    const char* separate;           // ro.separate.soft
    const char* twversion;          // ro.twrp.device_version
};

const std::unordered_map<std::string, ModelInfo> kModelInfoMap = {
    {"20061", {"OPPO",    "OP4E5D",       "OPPO",     "PEDM00",       "PEDM00",     "OP4E5D",       "20061",    "OPPO_Find_X3"}                         }, // horee CN
    {"20161", {"OPPO",    "OP4EC1",       "OPPO",     "PENM00",       "PENM00",     "OP4EC1",       "20161",    "OPPO_Reno6_Pro+"}                      }, // jin CN
    {"20163", {"OPPO",    "OP4EC1",       "OPPO",     "PENM00",       "PENM00",     "OP4EC1",       "20163",    "OPPO_Reno6_Pro+_Conan_Edition"}        }, // jin_conan CN
    {"20351", {"OPPO",    "OP4F7FL1",     "OPPO",     "CPH2247",      "CPH2247",    "OP4F7FL1",     "20351",    "OPPO_Reno6_Pro"}                       }, // jin EU
    {"20352", {"OPPO",    "OP4F7FL1",     "OPPO",     "CPH2247",      "CPH2247",    "OP4F7FL1",     "20352",    "OPPO_Reno6_Pro"}                       }, // jin GLO
    {"21615", {"realme",  "RE546F",       "realme",   "RMX3366",      "RMX3366",    "RE546F",       "21615",    "realme_GT_Master_Explorer_Edition"}    }, // rivena CN
    {"21619", {"realme",  "RE5473",       "realme",   "RMX3370",      "RMX3370",    "RE5473",       "21619",    "realme_GT_Neo_2"}                      }, // bitra CN
    {"2161A", {"realme",  "RE5473",       "realme",   "RMX3370",      "RMX3370",    "RE5473",       "2161A",    "realme_GT_Neo_2_Dragon_Ball_Edition"}  }, // bitra_dragon_ball CN
    {"2169A", {"realme",  "RE879AL1",     "realme",   "RMX3370",      "RMX3370",    "RE879AL1",     "2169A",    "realme_GT_Neo_2"}                      }, // bitra IN
    {"2169B", {"realme",  "RE879AL1",     "realme",   "RMX3370",      "RMX3370",    "RE879AL1",     "2169B",    "realme_GT_Neo_2"}                      }, // bitra EU
    {"0",     {"OPLUS",   "SM8250-AC",    "OPLUS",    "SM8250-AC",    "SM8250-AC",  "SM8250-AC",    "0",        "SM8250-AC"}                            }, // Default
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
    std::string name = info.base_name;
    struct PropPair {
        const char* key;
        const char* value;
    } props[] = {
        {"ro.product.brand",            info.brand},
        {"ro.product.device",           info.device},
        {"ro.product.manufacturer",     info.manufacturer},
        {"ro.product.model",            info.model},
        {"ro.product.name",             name.c_str()},
        {"ro.commonsoft.ota",           info.commonsoft},
        {"ro.separate.soft",            info.separate},
        {"ro.twrp.device_version",      info.twversion},
    };
    for (const auto& p:props) {
        OverrideProperty(p.key, p.value);
    }
}

void vendor_load_properties() {
    auto prjname = GetProperty("ro.boot.prjname", "0");
    auto model_info = kModelInfoMap.find(prjname);

    // Handle unknown device models
    if (model_info == kModelInfoMap.end()) {
        LOG(ERROR) << "Unknown prjname: " << prjname << ", using default";
        model_info = kModelInfoMap.find("0");
    }

    SetupModelProperties(model_info->second);
}
