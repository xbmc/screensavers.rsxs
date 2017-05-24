#include "lattice/lattice.hh"
#include "addoncommon.h"

void CMyAddon::SetSettings()
{
  int val;

  static float shininess = 50.0f;
  static bool sphereMap = false;
  static bool colored = true;
  static bool modulate = true;

  Hack::density = kodi::GetSettingInt("density");
  Hack::depth = kodi::GetSettingInt("depth");
  Hack::fog = kodi::GetSettingBoolean("fog");
  Hack::latitude = kodi::GetSettingInt("latitude");
  Hack::longitude = kodi::GetSettingInt("longitude");
  Hack::thickness = static_cast<float>(kodi::GetSettingInt("thickness"));
  Hack::smooth = kodi::GetSettingBoolean("smooth");

  val = kodi::GetSettingInt("surface");
  if (val == 0)
    val = (rand() % 8) + 1;
  switch (val)
  {
    case 1:
      Hack::linkType = Hack::SOLID_LINKS;
      shininess = 0.0f;
      sphereMap = false;
      colored = false;
      modulate = true;
      {
        Hack::Texture texture = { "industrial1.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      {
        Hack::Texture texture = { "industrial2.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 2:
      Hack::linkType = Hack::TRANSLUCENT_LINKS;
      shininess = 10.0f;
      sphereMap = true;
      colored = false;
      modulate = true;
      {
        Hack::Texture texture = { "crystal.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 3:
      Hack::linkType = Hack::SOLID_LINKS;
      shininess = -1.0f;
      sphereMap = true;
      colored = false;
      modulate = true;
      {
        Hack::Texture texture = { "chrome.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 4:
      Hack::linkType = Hack::SOLID_LINKS;
      shininess = -1.0f;
      sphereMap = true;
      colored = false;
      modulate = true;
      {
        Hack::Texture texture = { "brass.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 5:
      Hack::linkType = Hack::SOLID_LINKS;
      shininess = 50.0f;
      sphereMap = true;
      colored = true;
      modulate = false;
      {
        Hack::Texture texture = { "shiny.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 6:
      Hack::linkType = Hack::TRANSLUCENT_LINKS;
      shininess = -1.0f;
      sphereMap = true;
      colored = true;
      modulate = true;
      {
        Hack::Texture texture = { "ghostly.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 7:
      Hack::linkType = Hack::HOLLOW_LINKS;
      shininess = 50.0f;
      sphereMap = false;
      colored = true;
      modulate = true;
      {
        Hack::Texture texture = { "circuits.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
    case 8:
      Hack::linkType = Hack::SOLID_LINKS;
      shininess = 50.0f;
      sphereMap = false;
      colored = true;
      modulate = false;
      {
        Hack::Texture texture = { "doughnuts.png",
          shininess, sphereMap, colored, modulate };
        Hack::textures.push_back(texture);
      }
      break;
  }

  val = kodi::GetSettingInt("linktype");
  switch (val)
  {
    case 1:
      Hack::linkType = Hack::SOLID_LINKS;
      break;
    case 2:
      Hack::linkType = Hack::TRANSLUCENT_LINKS;
      break;
    case 3:
      Hack::linkType = Hack::HOLLOW_LINKS;
    default:
      break;
  }
}
