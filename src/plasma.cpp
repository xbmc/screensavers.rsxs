#include "plasma/plasma.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::zoom = static_cast<float>(kodi::GetSettingInt("zoom"));
  Hack::focus = static_cast<float>(kodi::GetSettingInt("focus"));
  Hack::speed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::resolution = kodi::GetSettingInt("resolution");
}
