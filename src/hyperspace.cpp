#include "hyperspace/hyperspace.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numStars = kodi::GetSettingInt("stars");
  Hack::starSize = static_cast<float>(kodi::GetSettingInt("size"));
  Hack::depth = kodi::GetSettingInt("depth");
  Hack::fov = static_cast<float>(kodi::GetSettingInt("fov"));
  Hack::speed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::resolution = kodi::GetSettingInt("resolution");
  Hack::shaders = kodi::GetSettingBoolean("shaders");
}
