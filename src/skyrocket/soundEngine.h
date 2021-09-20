/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 1999-2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#pragma once

#include <rsMath/rsMath.h>
#include <math.h>


#define NUM_SOUNDNODES 100
#define NUM_SOURCES 16  // 16 is the maximum that works on my computer
#define NUM_BUFFERS 10

#define LAUNCH1SOUND 0
#define LAUNCH2SOUND 1
#define BOOM1SOUND 2
#define BOOM2SOUND 3
#define BOOM3SOUND 4
#define BOOM4SOUND 5
#define POPPERSOUND 6
#define SUCKSOUND 7
#define NUKESOUND 8
#define WHISTLESOUND 9

namespace kodi
{
  namespace audioengine
  {
    class CAddonAEStream;
  }
}

class CSoundEngine
{
public:
// 	ALCcontext* context;
// 	ALCdevice* device;
// 	ALuint buffers[NUM_BUFFERS];
// 	ALuint sources[NUM_SOURCES];

  class SoundNode
  {
  public:
    int sound;
    float pos[3];
    float dist;
    float time;  // time until sound plays
    bool active;

    SoundNode(){active = false;}
    ~SoundNode(){}
  };
  SoundNode soundnodes[NUM_SOUNDNODES];

  CSoundEngine(/*HWND hwnd,*/ float volume);
  ~CSoundEngine();
  void insertSoundNode(int sound, rsVec source, rsVec observer);
  void update(float* listenerPos, float* listenerVel, float* listenerOri, float frameTime, bool slowMotion);

private:
  kodi::audioengine::CAddonAEStream* m_stream1;
};
