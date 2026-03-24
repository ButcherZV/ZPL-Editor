#include "AppConfig.h"
#include <wx/config.h>

AppConfig AppConfig::Load()
{
    AppConfig cfg;
    wxConfig config("ZPLEditor");
    int units = static_cast<int>(MeasureUnit::Metric);
    config.Read("/General/Units", &units, units);
    cfg.units = static_cast<MeasureUnit>(units);
    return cfg;
}

void AppConfig::Save() const
{
    wxConfig config("ZPLEditor");
    config.Write("/General/Units", static_cast<int>(units));
    config.Flush();
}
