#include "solarwinds/solarwinds.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numWinds = kodi::GetSettingInt("winds");
  Hack::numParticles = kodi::GetSettingInt("particles");
  Hack::numEmitters = kodi::GetSettingInt("emitters");
  Hack::windSpeed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::size = static_cast<float>(kodi::GetSettingInt("psize"));
  Hack::particleSpeed = static_cast<float>(kodi::GetSettingInt("pspeed"));
  Hack::emitterSpeed = static_cast<float>(kodi::GetSettingInt("espeed"));
  Hack::blur = static_cast<float>(kodi::GetSettingInt("blur"));

  int val = kodi::GetSettingInt("pgeom");
  if (val == 0)
    Hack::geometry = Hack::LIGHTS_GEOMETRY;
  else if (val == 1)
    Hack::geometry = Hack::POINTS_GEOMETRY;
  else if (val == 2)
    Hack::geometry = Hack::LINES_GEOMETRY;
}
