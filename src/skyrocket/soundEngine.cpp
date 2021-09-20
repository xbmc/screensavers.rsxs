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

#include "soundEngine.h"
#include "launchsound.h"
#include "boomsound.h"
#include "poppersound.h"
#include "sucksound.h"
#include "nukesound.h"
#include "whistlesound.h"

// #include <kodi/AudioEngine.h>

// sound is about halfway attenuated at reference distance
static float reference_distance[NUM_BUFFERS] =
{
  10.0f,  // launch sounds
  10.0f,
  1000.0f,  // booms
  1000.0f,
  1000.0f,
  1200.0f,
  700.0f,  // poppers
  1500.0f,  // suck
  2000.0f,  // nuke
  700.0f  // whistle
};


CSoundEngine::CSoundEngine(/*HWND hwnd,*/ float volume)
{
//   AudioEngineFormat format;
//   format.m_dataFormat = AE_FMT_S16BE;
//   format.m_sampleRate = 44100;
//   format.m_channelCount = 1;
//   format.m_channels[0] = AE_CH_FL;
//   m_stream1 = new kodi::audioengine::CAddonAEStream(format);
//   // Open device
//   device = alcOpenDevice((ALCchar*)"Generic Software");  // might work if NULL doesn't due to sound driver problems
//    //device = alcOpenDevice(NULL);  // default device
//   if(device == NULL)
//     return;
//   // Create context
//   context = alcCreateContext(device, NULL);
//   if(context == NULL)
//     return;
//   // Set active context
//   alcMakeContextCurrent(context);
//
//   alDistanceModel(AL_INVERSE_DISTANCE);
//   alDopplerVelocity(1130.0f);  // Sound travels at 1130 feet/sec
//   alListenerf(AL_GAIN, volume);  // Volume
//
//   // Initialize sound data
//   alGenBuffers(NUM_BUFFERS, buffers);
//   alBufferData(buffers[LAUNCH1SOUND], AL_FORMAT_MONO16, launch1SoundData, launch1SoundSize, 44100);
//   alBufferData(buffers[LAUNCH2SOUND], AL_FORMAT_MONO16, launch2SoundData, launch2SoundSize, 44100);
//   alBufferData(buffers[BOOM1SOUND], AL_FORMAT_MONO16, boom1SoundData, boom1SoundSize, 44100);
//   alBufferData(buffers[BOOM2SOUND], AL_FORMAT_MONO16, boom2SoundData, boom2SoundSize, 44100);
//   alBufferData(buffers[BOOM3SOUND], AL_FORMAT_MONO16, boom3SoundData, boom3SoundSize, 44100);
//   alBufferData(buffers[BOOM4SOUND], AL_FORMAT_MONO16, boom4SoundData, boom4SoundSize, 44100);
//   alBufferData(buffers[POPPERSOUND], AL_FORMAT_MONO16, popperSoundData, popperSoundSize, 44100);
//   alBufferData(buffers[SUCKSOUND], AL_FORMAT_MONO16, suckSoundData, suckSoundSize, 44100);
//   alBufferData(buffers[NUKESOUND], AL_FORMAT_MONO16, nukeSoundData, nukeSoundSize, 44100);
//   alBufferData(buffers[WHISTLESOUND], AL_FORMAT_MONO16, whistleSoundData, whistleSoundSize, 44100);
//
//   alGenSources(NUM_SOURCES, sources);
//   for(int i=0; i<NUM_SOURCES; ++i){
//     alSourcef(sources[i], AL_GAIN, 1.0f);
//     alSourcef(sources[i], AL_ROLLOFF_FACTOR, 1.0f);
//     alSourcei(sources[i], AL_LOOPING, AL_FALSE);
//   }
}


CSoundEngine::~CSoundEngine()
{
//   delete m_stream1;
//   alDeleteBuffers(NUM_BUFFERS, buffers);
//   alDeleteSources(NUM_SOURCES, sources);
//   //Release context
//   alcDestroyContext(context);
//   //Close device
//   alcCloseDevice(device);
}


void CSoundEngine::insertSoundNode(int sound, rsVec source, rsVec observer)
{
  rsVec dir = observer - source;

  // find an available SoundNode
  int index = -1;
  int i = 0;
  while(index < 0 && i < NUM_SOUNDNODES){
    if(soundnodes[i].active == false)
      index = i;
    ++i;
  }

  // escape if no SoundNode is available
  if(index == -1)
    return;

  soundnodes[index].sound = sound;
  if(soundnodes[index].sound == POPPERSOUND)  // poppers have a little delay
    soundnodes[index].time += 2.5f;
  soundnodes[index].pos[0] = source[0];
  soundnodes[index].pos[1] = source[1];
  soundnodes[index].pos[2] = source[2];
  // distance to sound
  soundnodes[index].dist = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
  // Sound travels at 1130 feet/sec
  soundnodes[index].time = soundnodes[index].dist * 0.000885f;
  soundnodes[index].active = true;
}


void CSoundEngine::update(float* listenerPos, float* listenerVel, float* listenerOri, float frameTime, bool slowMotion)
{
//     unsigned int AddData(uint8_t* const *data, unsigned int offset, unsigned int frames,
//                          double pts = 0, bool hasDownmix = false, double centerMixLevel = 1.0)

//   if(device == NULL || context == NULL)
//     return;
//
//   // Set current listener attributes
//   alListenerfv(AL_POSITION, listenerPos);
//   alListenerfv(AL_VELOCITY, listenerVel);
//   alListenerfv(AL_ORIENTATION, listenerOri);
//
  for(int i=0; i<NUM_SOUNDNODES; ++i)
  {
    if(soundnodes[i].active == true)
    {
      soundnodes[i].time -= frameTime;
      if(soundnodes[i].time <= 0.0f)
      {
        // find an available source
//         ALint value;
//         int src_index = -1;
//         int si = 0;
//         while(si < NUM_SOURCES && src_index == -1){
//           alGetSourcei(sources[si], AL_SOURCE_STATE, &value);
//           if(value != AL_PLAYING)
//             src_index = si;
//           ++si;
//         }
//         // play sound if a source is available
//         if(src_index > -1){
//           alSourcei(sources[src_index], AL_BUFFER, buffers[soundnodes[i].sound]);
//           alSourcef(sources[src_index], AL_REFERENCE_DISTANCE, reference_distance[soundnodes[i].sound]);
//           alSourcefv(sources[src_index], AL_POSITION, soundnodes[i].pos);
//           if(slowMotion)  // Slow down the sound
//             alSourcef(sources[src_index], AL_PITCH, 0.5f);
//           else  // Sound at regular speed
//             alSourcef(sources[src_index], AL_PITCH, 1.0f);
//           alSourcePlay(sources[src_index]);
//         }
//         // deactivate the SoundNode
//         soundnodes[i].active = false;
      }
    }
  }
}
