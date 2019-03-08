/*
 * Copyright (C) 2009 Tugrul Galatali <tugrul@galatali.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include <string>
#include <pthread.h>
#include <dirent.h>
#include <stdlib.h>

class TexMgr
{
  public:
    TexMgr();
    ~TexMgr();

    void setImageDir(const char *newDirName);
    void setTexSize(const unsigned int &w, const unsigned int &h) { tw = w; th = h; }
    void setGenTexSize(const unsigned int &w, const unsigned int &h) { gw = w; gh = h; }

    void start();
    void stop();

    bool getNext();
    unsigned int *getCurTex() const { return curTex; }
    unsigned int getCurW() const { return curW; }
    unsigned int getCurH() const { return curH; }
    unsigned int *getPrevTex() const { return prevTex; }
    unsigned int getPrevW() const { return prevW; }
    unsigned int getPrevH() const { return prevH; }

  private:
    int tw, th;
    unsigned int *prevTex;
    unsigned int prevW, prevH;
    unsigned int *curTex;
    unsigned int curW, curH;
    unsigned int *nextTex;
    unsigned int nextW, nextH;
    bool ready;

    std::string dirName;
    DIR *imageDir;

    pthread_t *imageThread;
    pthread_mutex_t *nextTexMutex;
    pthread_cond_t *nextTexCond;
    volatile bool exiting;

    unsigned int gw, gh;
    void genTex();

    void loadNextImageFromDisk();

    static void *imageThreadMain(void *vp);
};
