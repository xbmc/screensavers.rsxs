#pragma once

#include <kodi/addon-instance/Screensaver.h>

#include "hack.hh"

class CMyAddon
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver
{
public:
  CMyAddon();

  virtual bool Start() override;
  virtual void Stop() override;
  virtual void Render() override;

private:
  void SetSettings();
};

CMyAddon::CMyAddon()
{
  Common::width = Width();
  Common::height = Height();
  Common::aspectRatio = float(Common::width) / float(Common::height);
  Common::init(0, NULL);

  Common::resourceDir = kodi::GetAddonPath() + "/resources/";

  SetSettings();
}

bool CMyAddon::Start()
{
  Hack::start();
  return true;
}

void CMyAddon::Stop()
{
  Hack::stop();
}

void CMyAddon::Render()
{
  Hack::tick();
}

ADDONCREATOR(CMyAddon);
