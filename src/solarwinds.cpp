#include "solarwinds/solarwinds.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "winds") == 0)
    Hack::numWinds = *(int*)(settingValue);
  if (strcmp(settingName, "particles") == 0)
    Hack::numParticles = *(int*)(settingValue);
  if (strcmp(settingName, "emitters") == 0)
    Hack::numEmitters = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::windSpeed = *(int*)(settingValue);
  if (strcmp(settingName, "psize") == 0)
    Hack::size = *(int*)(settingValue);
  if (strcmp(settingName, "pspeed") == 0)
    Hack::particleSpeed = *(int*)(settingValue);
  if (strcmp(settingName, "espeed") == 0)
    Hack::emitterSpeed = *(int*)(settingValue);
  if (strcmp(settingName, "blur") == 0)
    Hack::blur = *(int*)(settingValue);

  if (strcmp(settingName, "pgeom") == 0)
  {
    if (*(int*)settingValue == 0)
      Hack::geometry = Hack::LIGHTS_GEOMETRY;
    if (*(int*)settingValue == 1)
      Hack::geometry = Hack::POINTS_GEOMETRY;
    if (*(int*)settingValue == 2)
      Hack::geometry = Hack::LINES_GEOMETRY;
  }

  return ADDON_STATUS_OK;
}

}
