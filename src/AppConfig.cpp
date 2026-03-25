#include "AppConfig.h"
#include <wx/config.h>

AppConfig AppConfig::Load()
{
    AppConfig cfg;
    wxConfig config("ZPLEditor");
    int units = static_cast<int>(MeasureUnit::Metric);
    config.Read("/General/Units", &units, units);
    cfg.units = static_cast<MeasureUnit>(units);
    config.Read("/General/SnapSize", &cfg.snapSize, cfg.snapSize);
    if (cfg.snapSize < 1) cfg.snapSize = 1;
    return cfg;
}

void AppConfig::Save() const
{
    wxConfig config("ZPLEditor");
    config.Write("/General/Units", static_cast<int>(units));
    config.Write("/General/SnapSize", snapSize);
    config.Flush();
}
