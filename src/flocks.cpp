#include "flocks/flocks.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "leaders") == 0)
    Hack::numLeaders = *(int*)(settingValue);
  if (strcmp(settingName, "followers") == 0)
    Hack::numFollowers = *(int*)(settingValue);
  if (strcmp(settingName, "size") == 0)
    Hack::size = *(int*)(settingValue);
  if (strcmp(settingName, "stretch") == 0)
    Hack::stretch = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::speed = *(int*)(settingValue);
  if (strcmp(settingName, "complexity") == 0)
    Hack::complexity = *(int*)(settingValue);
  if (strcmp(settingName, "cspeed") == 0)
    Hack::colorFadeSpeed = *(int*)(settingValue);
  if (strcmp(settingName, "cdepth") == 0)
    Hack::chromatek = *(bool*)(settingValue);
  if (strcmp(settingName, "connections") == 0)
    Hack::connections = *(bool*)(settingValue);

  Hack::blobs = Hack::complexity != 0;

  return ADDON_STATUS_OK;
}

}
