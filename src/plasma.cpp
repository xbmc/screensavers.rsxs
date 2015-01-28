#include "plasma/plasma.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "zoom") == 0)
    Hack::zoom = *(int*)(settingValue);
  if (strcmp(settingName, "focus") == 0)
    Hack::focus = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::speed = *(int*)(settingValue);
  if (strcmp(settingName, "resolution") == 0)
    Hack::resolution = *(int*)(settingValue);

  return ADDON_STATUS_OK;
}
}
