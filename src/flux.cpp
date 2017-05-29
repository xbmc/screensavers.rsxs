#include "flux/flux.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numFluxes = kodi::GetSettingInt("fluxes");
  Hack::numTrails = kodi::GetSettingInt("particles");
  Hack::trailLength = kodi::GetSettingInt("length");
  Hack::complexity = kodi::GetSettingInt("complexity");
  Hack::trailLength = kodi::GetSettingInt("speed");
  Hack::randomize = kodi::GetSettingInt("randomness");
  Hack::rotation = static_cast<float>(kodi::GetSettingInt("rotation"));
  Hack::wind = static_cast<float>(kodi::GetSettingInt("wind"));
  Hack::instability = static_cast<float>(kodi::GetSettingInt("instability"));
  Hack::blur = static_cast<float>(kodi::GetSettingInt("blur"));
  int val = kodi::GetSettingInt("pgeom");
  if (val == 0)
    Hack::geometry = Hack::LIGHTS_GEOMETRY;
  else if (val == 1)
    Hack::geometry = Hack::POINTS_GEOMETRY;
  else if (val == 2)
    Hack::geometry = Hack::SPHERES_GEOMETRY;
}
