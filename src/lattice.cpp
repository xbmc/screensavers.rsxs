#include "lattice/lattice.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "density") == 0)
    Hack::density = *(int*)(settingValue);
  if (strcmp(settingName, "depth") == 0)
    Hack::depth = *(int*)(settingValue);
  if (strcmp(settingName, "fog") == 0)
    Hack::fog = *(bool*)(settingValue);
  if (strcmp(settingName, "latitude") == 0)
    Hack::latitude = *(int*)(settingValue);
  if (strcmp(settingName, "longitude") == 0)
    Hack::longitude = *(int*)(settingValue);
  if (strcmp(settingName, "thickness") == 0)
    Hack::thickness = *(int*)(settingValue);
  if (strcmp(settingName, "smooth") == 0)
    Hack::thickness = *(bool*)(settingValue);

  return ADDON_STATUS_OK;
}

}
