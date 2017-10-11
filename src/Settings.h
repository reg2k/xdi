#pragma once

namespace Settings
{
    using std::string;

    int     GetInt(std::string settingName, int default = 0);
    bool    GetBool(std::string settingName, bool default = false);
    float   GetFloat(std::string settingName, float default = 0.0f);
    string  GetString(std::string settingName, string default = "");
}