#include "skyrocket/skyrocket.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "rockets") == 0)
    Hack::maxRockets = *(int*)(settingValue);
  if (strcmp(settingName, "smoke") == 0)
    Hack::smoke = *(int*)(settingValue);
  if (strcmp(settingName, "esmoke") == 0)
    Hack::explosionSmoke = *(int*)(settingValue);
  if (strcmp(settingName, "moon") == 0)
    Hack::drawMoon = *(bool*)(settingValue);
  if (strcmp(settingName, "clouds") == 0)
    Hack::drawClouds = *(bool*)(settingValue);
  if (strcmp(settingName, "earth") == 0)
    Hack::drawEarth = *(bool*)(settingValue);
  if (strcmp(settingName, "glow") == 0)
    Hack::drawIllumination = *(bool*)(settingValue);
  if (strcmp(settingName, "stars") == 0)
    Hack::starDensity = *(int*)(settingValue);
  if (strcmp(settingName, "halo") == 0)
    Hack::moonGlow = *(int*)(settingValue);
  if (strcmp(settingName, "ambient") == 0)
    Hack::ambient = *(int*)(settingValue);
  if (strcmp(settingName, "wind") == 0)
    Hack::wind = *(int*)(settingValue);
  if (strcmp(settingName, "flares") == 0)
    Hack::flares = *(int*)(settingValue);

  return ADDON_STATUS_OK;
}

}
