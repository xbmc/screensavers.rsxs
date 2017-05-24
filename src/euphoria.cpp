#include "euphoria/euphoria.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numWisps = kodi::GetSettingInt("wisps");
  Hack::numBackWisps = kodi::GetSettingInt("background");
  Hack::density = kodi::GetSettingInt("density");
  Hack::speed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::feedback = static_cast<float>(kodi::GetSettingInt("feedback"));
  Hack::feedbackSpeed = static_cast<float>(kodi::GetSettingInt("fspeed"));
  Hack::feedbackSize = kodi::GetSettingInt("fsize");

  int val = kodi::GetSettingInt("texture");
  if (val == 4)
    val = rand()%4;
  if (val == 1)
    Hack::texture = "plasma.png";
  else if (val == 2)
    Hack::texture = "stringy.png";
  else if (val == 3)
    Hack::texture = "lines.png";
}
