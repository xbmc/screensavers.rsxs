#include "flux/flux.hh"
#include "addoncommon.h"

extern "C" {

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  if (strcmp(settingName, "fluxes") == 0)
    Hack::numFluxes = *(int*)(settingValue);
  if (strcmp(settingName, "particles") == 0)
    Hack::numTrails = *(int*)(settingValue);
  if (strcmp(settingName, "length") == 0)
    Hack::trailLength = *(int*)(settingValue);
  if (strcmp(settingName, "complexity") == 0)
    Hack::complexity = *(int*)(settingValue);
  if (strcmp(settingName, "speed") == 0)
    Hack::expansion = *(int*)(settingValue);
  if (strcmp(settingName, "randomness") == 0)
    Hack::randomize = *(int*)(settingValue);
  if (strcmp(settingName, "rotation") == 0)
    Hack::rotation = *(int*)(settingValue);
  if (strcmp(settingName, "wind") == 0)
    Hack::wind = *(int*)(settingValue);
  if (strcmp(settingName, "instability") == 0)
    Hack::instability = *(int*)(settingValue);
  if (strcmp(settingName, "blur") == 0)
    Hack::blur = *(int*)(settingValue);
  if (strcmp(settingName, "pgeom") == 0)
  {
    if (*(int*)settingValue == 0)
      Hack::geometry = Hack::LIGHTS_GEOMETRY;
    if (*(int*)settingValue == 1)
      Hack::geometry = Hack::POINTS_GEOMETRY;
    if (*(int*)settingValue == 2)
      Hack::geometry = Hack::SPHERES_GEOMETRY;
  }

  return ADDON_STATUS_OK;
}

}
