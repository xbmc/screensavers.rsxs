#include "flocks/flocks.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numLeaders = kodi::GetSettingInt("leaders");
  Hack::numFollowers = kodi::GetSettingInt("followers");
  Hack::size = static_cast<float>(kodi::GetSettingInt("size"));
  Hack::stretch = static_cast<float>(kodi::GetSettingInt("stretch"));
  Hack::speed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::complexity = kodi::GetSettingInt("complexity");
  Hack::colorFadeSpeed = static_cast<float>(kodi::GetSettingInt("cspeed"));
  Hack::chromatek = kodi::GetSettingBoolean("cdepth");
  Hack::connections = kodi::GetSettingBoolean("connections");

  Hack::blobs = Hack::complexity != 0;
}
