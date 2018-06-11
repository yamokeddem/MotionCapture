#ifndef  SOUND_EFFECT_H
#define  SOUND_EFFECT_H

#include <SDL2/SDL_mixer.h>

#define SOUND_EFFECT_BIP    0
#define LOOP_INFINITE       -1
#define LOOP_ONCE           1

extern int initSoundEffect();
extern void playSound(int soundId, int loop);

#endif
