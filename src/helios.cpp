#include "helios/helios.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "ions") == 0)
    Hack::numIons = *(int*)(settingValue);
  if (strcmp(settingName, "emitters") == 0)
    Hack::numEmitters = *(int*)(settingValue);
  if (strcmp(settingName, "attractors") == 0)
    Hack::numAttractors = *(int*)(settingValue);
  if (strcmp(settingName, "size") == 0)
    Hack::size = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::speed = *(int*)(settingValue);
  if (strcmp(settingName, "isosurface") == 0)
    Hack::surface = *(bool*)(settingValue);
  if (strcmp(settingName, "wireframe") == 0)
    Hack::wireframe = *(bool*)(settingValue);
  if (strcmp(settingName, "cspeed") == 0)
    Hack::cameraSpeed = *(int*)(settingValue);
  if (strcmp(settingName, "blur") == 0)
    Hack::blur = *(int*)(settingValue);

  return ADDON_STATUS_OK;
}

}
