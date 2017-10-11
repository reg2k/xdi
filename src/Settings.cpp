#include "Settings.h"

#include "Config.h"

#include <string>

namespace Settings {

    std::string Get_Internal(std::string settingNameAndSection) {
        auto delimiter = settingNameAndSection.find_first_of(':');
        std::string settingNameStr     = settingNameAndSection.substr(0, delimiter);
        std::string settingSectionStr  = settingNameAndSection.substr(delimiter + 1);

        char value[1024];
        GetPrivateProfileString(settingSectionStr.c_str(), settingNameStr.c_str(), NULL, value, sizeof(value), INI_LOCATION_USER);

        if (strlen(value) == 0) {
            GetPrivateProfileString(settingSectionStr.c_str(), settingNameStr.c_str(), NULL, value, sizeof(value), INI_LOCATION_DEFAULTS);
        }

        return std::string(value);
    }

    int GetInt(std::string settingName, int default) {
        auto value = Get_Internal(settingName);
        return (value != "") ? std::stoi(value) : default;
    }

    bool GetBool(std::string settingName, bool default) {
        auto value = Get_Internal(settingName);
        return (value != "") ? (std::stoi(value) != 0) : default;
    }

    float GetFloat(std::string settingName, float default) {
        auto value = Get_Internal(settingName);
        return (value != "") ? std::stof(value) : default;
    }

    std::string GetString(std::string settingName, std::string default) {
        auto value = Get_Internal(settingName);
        return (value != "") ? value : default;
    }
}