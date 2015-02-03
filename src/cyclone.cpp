#include "cyclone/cyclone.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "cyclones") == 0)
    Hack::numCyclones = *(int*)(settingValue);
  if (strcmp(settingName, "complexity") == 0)
    Hack::complexity = *(int*)(settingValue);
  if (strcmp(settingName, "particles") == 0)
    Hack::numParticles = *(int*)(settingValue);
  if (strcmp(settingName, "curves") == 0)
    Hack::showCurves = *(bool*)(settingValue);
  if (strcmp(settingName, "hemisphere") == 0)
  {
    if (*(int*)settingValue == 0)
     Hack::southern = false;
    if (*(int*)settingValue == 1)
     Hack::southern = true;
  }
  if (strcmp(settingName, "size") == 0)
    Hack::size = *(int*)(settingValue);
  if (strcmp(settingName, "stretch") == 0)
    Hack::stretch = *(bool*)(settingValue);

  return ADDON_STATUS_OK;
}
}
