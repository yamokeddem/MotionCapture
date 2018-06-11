#include "SoundEffect.h"
#include "Logger.h"

#define NUMBER_SOUND_EFFECT 1
Mix_Chunk * soundEffect[NUMBER_SOUND_EFFECT]; // Canaux des sounds effects


/*!\brief Fonction qui initialise les paramètres SDL_Mixer et charge
 *  les fichier audio */
int initSoundEffect()
{
    char *file[] = {"SoundEffect/bipTime.wav"};
    int i;
    
    Mix_AllocateChannels(3);
    if(Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0)
        exit(-4);
    
    for(i=0 ; i<NUMBER_SOUND_EFFECT ; i++)
    {
        if(!(soundEffect[i] = Mix_LoadWAV(file[i])))
        {
            fprintf(stderr, "Erreur lors du Mix_LoadMUS: %s\n", Mix_GetError());
            return -1;
        }
    }
    
    LOG4CXX_INFO(GlobalLogger, "Chargement des effest sonores !");
    return 0;
}

/*!\brief Fonction qui joue un effet sonore (une fois, ou à l'infinie) */
void playSound(int soundId, int loop)
{
    if(loop != -1)
        loop -= 1;
    Mix_PlayChannel(soundId, soundEffect[soundId], loop);
}
