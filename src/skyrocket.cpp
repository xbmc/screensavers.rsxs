#include "skyrocket/skyrocket.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::maxRockets = kodi::GetSettingInt("rockets");
  Hack::smoke = static_cast<float>(kodi::GetSettingInt("smoke"));
  Hack::explosionSmoke = kodi::GetSettingInt("esmoke");
  Hack::drawMoon = kodi::GetSettingBoolean("moon");
  Hack::drawClouds = kodi::GetSettingBoolean("clouds");
  Hack::drawEarth = kodi::GetSettingBoolean("earth");
  Hack::drawIllumination = kodi::GetSettingBoolean("glow");
  Hack::starDensity = kodi::GetSettingInt("stars");
  Hack::moonGlow = static_cast<float>(kodi::GetSettingInt("halo"));
  Hack::ambient = static_cast<float>(kodi::GetSettingInt("ambient"));
  Hack::wind = static_cast<float>(kodi::GetSettingInt("wind"));
  Hack::flares = static_cast<float>(kodi::GetSettingInt("flares"));
}
