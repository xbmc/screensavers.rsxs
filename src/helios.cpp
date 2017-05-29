#include "helios/helios.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numIons = kodi::GetSettingInt("ions");
  Hack::numEmitters = kodi::GetSettingInt("emitters");
  Hack::numAttractors = kodi::GetSettingInt("attractors");
  Hack::size = static_cast<float>(kodi::GetSettingInt("size"));
  Hack::speed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::surface = kodi::GetSettingBoolean("isosurface");
  Hack::wireframe = kodi::GetSettingBoolean("wireframe");
  Hack::cameraSpeed = static_cast<float>(kodi::GetSettingInt("cspeed"));
  Hack::blur = static_cast<float>(kodi::GetSettingInt("blur"));
}
