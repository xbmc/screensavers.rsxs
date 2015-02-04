#include "fieldlines/fieldlines.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "ions") == 0)
    Hack::numIons = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::speed = *(int*)(settingValue);
  if (strcmp(settingName, "seqsize") == 0)
    Hack::stepSize = *(int*)(settingValue);
  if (strcmp(settingName, "numlines") == 0)
    Hack::maxSteps = *(int*)(settingValue);
  if (strcmp(settingName, "width") == 0)
    Hack::width = *(int*)(settingValue);
  if (strcmp(settingName, "constant") == 0)
    Hack::constWidth = *(bool*)(settingValue);
  if (strcmp(settingName, "electric") == 0)
    Hack::electric = *(bool*)(settingValue);

  return ADDON_STATUS_OK;
}

}
