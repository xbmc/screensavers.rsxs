#include "euphoria/euphoria.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "wisps") == 0)
    Hack::numWisps = *(int*)(settingValue);
  if (strcmp(settingName, "background") == 0)
    Hack::numBackWisps = *(int*)(settingValue);
  if (strcmp(settingName, "density") == 0)
    Hack::density = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::speed = *(int*)(settingValue);
  if (strcmp(settingName, "feedback") == 0)
    Hack::feedback = *(int*)(settingValue);
  if (strcmp(settingName, "fspeed") == 0)
    Hack::feedbackSpeed = *(int*)(settingValue);
  if (strcmp(settingName, "fsize") == 0)
    Hack::feedbackSize = *(int*)(settingValue);
  if (strcmp(settingName, "texture") == 0)
  {
    int val = *(int*)settingValue;
    if (val == 4)
      val = rand()%4;
    if (val == 1)
      Hack::texture = "plasma.png";
    if (val == 2)
      Hack::texture = "stringy.png";
    if (val == 3)
      Hack::texture = "lines.png";
  }

  return ADDON_STATUS_OK;
}

}
