#include "hyperspace/hyperspace.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "stars") == 0)
    Hack::numStars = *(int*)(settingValue);
  if (strcmp(settingName, "size") == 0)
    Hack::starSize = *(int*)(settingValue);
  if (strcmp(settingName, "depth") == 0)
    Hack::depth = *(int*)(settingValue);
  if (strcmp(settingName, "fov") == 0)
    Hack::fov = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::speed = *(int*)(settingValue);
  if (strcmp(settingName, "resolution") == 0)
    Hack::resolution = *(int*)(settingValue);
  if (strcmp(settingName, "shaders") == 0)
    Hack::shaders = *(bool*)(settingValue);

  return ADDON_STATUS_OK;
}

}
