/*
 *  Copyright (C) 2009 Tugrul Galatali <tugrul@galatali.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <condition_variable>
#include <dirent.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <mutex>

class TexMgr
{
  public:
    TexMgr();
    ~TexMgr();

    void setImageDir(const std::string& newDirName);
    void setTexSize(const unsigned int &w, const unsigned int &h) { m_tw = w; m_th = h; }
    void setGenTexSize(const unsigned int &w, const unsigned int &h) { m_gw = w; m_gh = h; }

    void start();
    void stop();

    bool getNext();
    unsigned int *getCurTex() const { return m_curTex; }
    unsigned int getCurW() const { return m_curW; }
    unsigned int getCurH() const { return m_curH; }
    unsigned int *getPrevTex() const { return m_prevTex; }
    unsigned int getPrevW() const { return m_prevW; }
    unsigned int getPrevH() const { return m_prevH; }

  private:
    int m_tw = -2;
    int m_th = -2;
    unsigned int* m_prevTex = nullptr;
    unsigned int m_prevW = 0;
    unsigned int m_prevH = 0;
    unsigned int* m_curTex = nullptr;
    unsigned int m_curW = 0;
    unsigned int m_curH = 0;
    unsigned int* m_nextTex = nullptr;
    unsigned int m_nextW = 0;
    unsigned int m_nextH = 0;
    bool m_ready = false;

    std::string m_dirName;
    DIR* m_imageDir = nullptr;

    std::thread m_imageThread;
    std::mutex m_nextTexMutex;
    std::condition_variable m_nextTexCond;
    volatile bool m_exiting = false;

    unsigned int m_gw = 256;
    unsigned int m_gh = 256;

    void genTex();

    void loadNextImageFromDisk();

    void imageThreadMain();
};
