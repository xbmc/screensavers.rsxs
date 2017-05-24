#include "cyclone/cyclone.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numCyclones = kodi::GetSettingInt("cyclones");
  Hack::complexity = kodi::GetSettingInt("complexity");
  Hack::numParticles = kodi::GetSettingInt("particles");
  Hack::showCurves = kodi::GetSettingBoolean("curves");
  Hack::southern = (kodi::GetSettingInt("hemisphere") == 0) ? false : true;
  Hack::size = static_cast<float>(kodi::GetSettingInt("size"));
  Hack::stretch = kodi::GetSettingBoolean("stretch");
}
