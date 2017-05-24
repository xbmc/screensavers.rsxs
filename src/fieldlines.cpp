#include "fieldlines/fieldlines.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  Hack::numIons = kodi::GetSettingInt("ions");
  Hack::speed = static_cast<float>(kodi::GetSettingInt("speed"));
  Hack::stepSize = static_cast<float>(kodi::GetSettingInt("seqsize"));
  Hack::maxSteps = kodi::GetSettingInt("numlines");
  Hack::width = static_cast<float>(kodi::GetSettingInt("width"));
  Hack::constWidth = kodi::GetSettingBoolean("constant");
  Hack::electric = kodi::GetSettingBoolean("electric");
}
