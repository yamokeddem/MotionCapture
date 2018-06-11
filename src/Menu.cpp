#include "Window.h"
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL2/SDL_image.h>
#include "Menu.h"
#include "assert.h"
#include "JointCoordinates.h"
#include "Logger.h"
#include "SoundEffect.h"

#include <time.h>

#define NUMBER_LAYOUTS          2
#define BUTTON_MOCAP            0
#define BUTTON_GAME             1

#define NUMBER_TEXTURES         6
#define TEXTURE_BACKGROUND      0
#define TEXTURE_BUTTON_MOCAP    1
#define TEXTURE_BUTTON_GAME     2
#define TEXTURE_NUMBER_1        3
#define TEXTURE_NUMBER_2        4
#define TEXTURE_NUMBER_3        5

struct timespec start, stop;
bool hooverButton[NUMBER_LAYOUTS] = {false};

GLuint _texturesId[NUMBER_TEXTURES];

const char *files[] = {"Pictures/Menu/background.jpg",
                       "Pictures/Menu/button1.png",
                       "Pictures/Menu/button2.png",
                       "Pictures/Menu/1.png",
                       "Pictures/Menu/2.png",
                       "Pictures/Menu/3.png"};

pLayout _layoutMenu[2];
GLuint quadBackground;
GLuint quadSablier;

/*!\brief Fonction qui initialise le Menu avec tous ses layouts*/
int initMenu()
{
    SDL_Surface *t;
    
    for(int i=0 ; i<NUMBER_LAYOUTS ; i++)
    {
        _layoutMenu[i] = (pLayout)malloc(sizeof(SLayout));
        assert(_layoutMenu[i]);
    }
    
    _layoutMenu[BUTTON_MOCAP]->surface = gl4dgGenQuadf();
    _layoutMenu[BUTTON_GAME]->surface = gl4dgGenQuadf();
    quadBackground = gl4dgGenQuadf();
    quadSablier = gl4dgGenQuadf();
    
    glGenTextures(NUMBER_TEXTURES, _texturesId);
    glEnable(GL_TEXTURE_2D);
    for(int i = 0; i < NUMBER_TEXTURES; i++)
    {
        glBindTexture(GL_TEXTURE_2D, _texturesId[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        if( (t = IMG_Load(files[i])) != NULL )
        {
#ifdef __APPLE__
            int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
            int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif
            
            if(i==0)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA , t->w, t->h, 0, GL_RGBA , GL_UNSIGNED_BYTE, t->pixels);
            
            SDL_FreeSurface(t);
        }
        else
        {
            fprintf(stderr, "can't open file %s : %s\n", files[i], SDL_GetError());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            return -1;
        }
    }
    
    setMenuLayoutDimensions();
    LOG4CXX_INFO(GlobalLogger, "Initialisation du menu reussie !");
    return 0;
}

/*!\brief Fonction qui test si un bouton a été cliqué ou non, le bouton est considéré comme clique si il est survolé pendant 3 secondes consécutives */
bool isClicked(int buttonId)
{
    int delay = 4;
    int x,y;
    static int soundBipCount[NUMBER_LAYOUTS];
    SDL_PumpEvents();
    SDL_GetMouseState(&x, &y);
    int buttonWidth  = _layoutMenu[buttonId]->width;
    int buttonHeight = _layoutMenu[buttonId]->height;
    int flecheX = _layoutMenu[buttonId]->x;
    int flecheY = _layoutMenu[buttonId]->y;
    
    if(!MAIN_DROITE->isAvailable())
    {
        int mainX = x;//MAIN_DROITE->x;
        int mainY = y;//MAIN_DROITE->y;
        
        if(mainX >= (flecheX-buttonWidth) && mainX <= (flecheX + buttonWidth) &&
           mainY >= (flecheY-buttonHeight) && mainY <= (flecheY + buttonHeight))
        {
            //Enregistrement du temp de depart
            if(hooverButton[buttonId] == false)
            {
                soundBipCount[buttonId] = 1;
                hooverButton[buttonId] = true;
                clock_gettime( CLOCK_REALTIME, &start);
            }
            
            zoomLayout(true, buttonId);
            
            //Si le temp d'arrivée - temp de depart >= delai alors return true
            clock_gettime( CLOCK_REALTIME, &stop);
            float timeElapsed = stop.tv_sec - start.tv_sec;

            if(timeElapsed >=1 && timeElapsed < 2)
            {
                drawNumbers(buttonId, TEXTURE_NUMBER_3);
                if(soundBipCount[buttonId] == 1)
                {
                    playSound(SOUND_EFFECT_BIP, LOOP_ONCE);
                    soundBipCount[buttonId] = 2;
                }
            }
            else if(timeElapsed >=2 && timeElapsed < 3)
            {
                drawNumbers(buttonId, TEXTURE_NUMBER_2);
                if(soundBipCount[buttonId] == 2)
                {
                    playSound(SOUND_EFFECT_BIP, LOOP_ONCE);
                    soundBipCount[buttonId] = 3;
                }
            }
            else if(timeElapsed >=3 && timeElapsed < 4)
            {
                drawNumbers(buttonId, TEXTURE_NUMBER_1);
                if(soundBipCount[buttonId] == 3)
                {
                    playSound(SOUND_EFFECT_BIP, LOOP_ONCE);
                    soundBipCount[buttonId] = 1;
                }
            }
            
            if(timeElapsed >= delay)
            {
                hooverButton[buttonId] = false;
                return true;
            }
            return false;
        }
    }

    if(MAIN_GAUCHE->isAvailable())
    {
        int mainX = MAIN_GAUCHE->x;
        int mainY = MAIN_GAUCHE->y;
        
         if(mainX >= (flecheX-buttonWidth) && mainX <= (flecheX + buttonWidth) &&
         mainY >= (flecheY-buttonHeight) && mainY <= (flecheY + buttonHeight))
         {
             //Enregistrement du temp de depart
             if(hooverButton[buttonId] == false)
             {
                 soundBipCount[buttonId] = 1;
                 hooverButton[buttonId] = true;
                 clock_gettime( CLOCK_REALTIME, &start);
             }
             
             zoomLayout(true, buttonId);
             
             float timeElapsed = stop.tv_sec - start.tv_sec;
             
             if(timeElapsed <= 1)
             {
                 drawNumbers(buttonId, TEXTURE_NUMBER_3);
                 if(soundBipCount[buttonId] == 1)
                 {
                     playSound(SOUND_EFFECT_BIP, LOOP_ONCE);
                     soundBipCount[buttonId] = 2;
                 }
             }
             else if(timeElapsed <= 2)
             {
                 drawNumbers(buttonId, TEXTURE_NUMBER_2);
                 if(soundBipCount[buttonId] == 2)
                 {
                     playSound(SOUND_EFFECT_BIP, LOOP_ONCE);
                     soundBipCount[buttonId] = 3;
                 }
             }
             else if(timeElapsed <= 3)
             {
                 drawNumbers(buttonId, TEXTURE_NUMBER_1);
                 if(soundBipCount[buttonId] == 3)
                 {
                     playSound(SOUND_EFFECT_BIP, LOOP_ONCE);
                     soundBipCount[buttonId] = 1;
                 }
             }
             
             //Si le temp d'arrivée - temp de depart >= delai alors return true
             clock_gettime( CLOCK_REALTIME, &stop);
             if(timeElapsed >= delay)
             {
                 hooverButton[buttonId] = false;
                 return true;
             }
             return false;
         }
    }

    soundBipCount[buttonId] = 1;
    zoomLayout(false, buttonId);
    hooverButton[buttonId] = false;
    return false;
}

/*!\brief Fonction qui effectue un zoom sur le bouton lorsque celui-ci est survolé par la souris */
void zoomLayout(bool zoom, int buttonId)
{
    if(zoom)
    {
        _layoutMenu[buttonId]->width = _windowWidth/5;
        _layoutMenu[buttonId]->height = _windowHeight/18;
    }
    else
    {
        _layoutMenu[buttonId]->width = _windowWidth/6;
        _layoutMenu[buttonId]->height = _windowHeight/20;
    }
}

/*!\brief Fonction qui définie les dimensions des layouts en fonction des dimensions de la fenêtre (utilisée dans le cas d'un redimensionnement de fenêtre) */
void setMenuLayoutDimensions()
{
    if(_layoutMenu[BUTTON_MOCAP] != NULL)
    {
        _layoutMenu[BUTTON_MOCAP]->x = _windowWidth/2;
        _layoutMenu[BUTTON_MOCAP]->y = _windowHeight/2.5;
        _layoutMenu[BUTTON_MOCAP]->width = _windowWidth/6;
        _layoutMenu[BUTTON_MOCAP]->height = _windowHeight/20;
    }
    
    if(_layoutMenu[BUTTON_GAME] != NULL)
    {
        _layoutMenu[BUTTON_GAME]->x = _windowWidth/2;
        _layoutMenu[BUTTON_GAME]->y = _windowHeight/1.7;
        _layoutMenu[BUTTON_GAME]->width = _windowWidth/6;
        _layoutMenu[BUTTON_GAME]->height = _windowHeight/20;
    }
}

/*!\brief Fonction qui affiche le numéro de seconde restente avant le lancement du mode */
void drawNumbers(int buttonId, int textureID)
{
    static float rotation;
    float vitesse = 4;
    float size = 15;
    
    rotation = 0;
    
    int x =_layoutMenu[buttonId]->x - _layoutMenu[buttonId]->width - 30;
    int y =_layoutMenu[buttonId]->y ;
    
    gl4duLoadIdentityf();
    gl4duTranslatef(x,y, -1);
    gl4duScalef(size,size,1);
    //gl4duRotatef(rotation,0,0,1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[textureID]);
    gl4dgDraw(quadSablier);
    
    x =_layoutMenu[buttonId]->x + _layoutMenu[buttonId]->width + 30;
    
    gl4duLoadIdentityf();
    gl4duTranslatef(x,y, -1);
    gl4duScalef(size,size,1);
    //gl4duRotatef(rotation,0,0,1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[textureID]);
    gl4dgDraw(quadSablier);
    
    rotation += vitesse;
}

/*!\brief Fonction qui affiche l'arrière plan "animé" du Menu  */
void drawBackgroundMenu()
{
    static float x1 = _windowWidth/2;
    static float x2 = _windowWidth + _windowWidth/2;
    static float vitesse = 1;
    
    //Draw Background
    gl4duLoadIdentityf();
    gl4duTranslatef(x1,_windowHeight/2, -11);
    gl4duScalef(_windowWidth/2, _windowHeight/2, 1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_BACKGROUND]);
    gl4dgDraw(quadBackground);
    
    //Draw Background
    gl4duLoadIdentityf();
    gl4duTranslatef(x2,_windowHeight/2, -11);
    gl4duScalef(_windowWidth/2, _windowHeight/2, 1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_BACKGROUND]);
    gl4dgDraw(quadBackground);
    
    if(x2 <= _windowWidth/2)
    {
        x1 = _windowWidth/2;
        x2 = _windowWidth + _windowWidth/2;
    }
    
    x1 -= vitesse;
    x2 -= vitesse;
}

/*!\brief Fonction d'affichage qui affiche l'intégralité du Menu et ses layouts */
void drawMenu()
{
    drawBackgroundMenu();
    
    //Draw Button MOCAP
    gl4duLoadIdentityf();
    gl4duTranslatef(_layoutMenu[BUTTON_MOCAP]->x, _layoutMenu[BUTTON_MOCAP]->y, -10);
    gl4duScalef(_layoutMenu[BUTTON_MOCAP]->width, _layoutMenu[BUTTON_MOCAP]->height, 1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_BUTTON_MOCAP]);
    gl4dgDraw(_layoutMenu[BUTTON_MOCAP]->surface);
    
    //Draw Button GAME
    gl4duLoadIdentityf();
    gl4duTranslatef(_layoutMenu[BUTTON_GAME]->x, _layoutMenu[BUTTON_GAME]->y, -10);
    gl4duScalef(_layoutMenu[BUTTON_GAME]->width, _layoutMenu[BUTTON_GAME]->height, 1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_BUTTON_GAME]);
    gl4dgDraw(_layoutMenu[BUTTON_GAME]->surface);
    
    if(isClicked(BUTTON_MOCAP))
    {
        LOG4CXX_INFO(GlobalLogger, "Lancement du mode Avatar Motion capture !");
        MODE = MOCAP;
        adaptTheLandmark();
        return;
    }
    
    if(isClicked(BUTTON_GAME))
    {
        LOG4CXX_INFO(GlobalLogger, "Lancement du Jeux d'avion !");
        MODE = GAME;
        adaptTheLandmark();
        return;
    }
}

/*!\brief Fonction qui libère la mémoire du mode Menu */
void freeMenu()
{
    LOG4CXX_INFO(GlobalLogger, "Destruction du Menu !");
    for(int i=0 ; i<NUMBER_TEXTURES ; i++)
        glDeleteTextures(NUMBER_TEXTURES, _texturesId);
    
    for(int i=0 ; i<NUMBER_LAYOUTS ; i++)
        if(_layoutMenu[i])
            free(_layoutMenu[i]);
}




