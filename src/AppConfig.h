#pragma once
#include "zpl/ZPLTypes.h"

// Singleton-style app configuration stored in wxConfig (registry on Windows).
// Access via AppConfig::Get().
struct AppConfig
{
    MeasureUnit units    = MeasureUnit::Metric;
    int         snapSize = 10;  // grid snap size in dots

    // Load from wxConfig
    static AppConfig Load();

    // Save to wxConfig
    void Save() const;

    // Global accessor (initialised in App::OnInit)
    static AppConfig& Get()
    {
        static AppConfig instance;
        return instance;
    }
};
