#pragma once

#include "xbmc_scr_dll.h"
#include "libXBMC_addon.h"
#include "hack.hh"

ADDON::CHelper_libXBMC_addon *XBMC           = NULL;

extern "C" {

ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!props)
    return ADDON_STATUS_UNKNOWN;

  if (!XBMC)
    XBMC = new ADDON::CHelper_libXBMC_addon;

  if (!XBMC->RegisterMe(hdl))
  {
    delete XBMC, XBMC=NULL;
    return ADDON_STATUS_PERMANENT_FAILURE;
  }

  SCR_PROPS* scrprops = (SCR_PROPS*)props;

  Common::width = scrprops->width;
  Common::height = scrprops->height;
  Common::aspectRatio = float(Common::width) / float(Common::height);
  Common::init(0, NULL);

  char temp[1024];
  XBMC->GetSetting("__addonpath__",temp);
  Common::resourceDir = temp;
  Common::resourceDir += "/resources/";

  return ADDON_STATUS_NEED_SETTINGS;
}

void Start()
{
  Hack::start();
}

void Render()
{
  Hack::tick();
}

void ADDON_Stop()
{
  Hack::stop();
  delete XBMC, XBMC=NULL;
}

void ADDON_Destroy()
{
}

ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

void Remove()
{
}

}
