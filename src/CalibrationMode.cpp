#include "CalibrationMode.h"
#include "Window.h"
#include <GL4D/gl4du.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include "Mocap.h"
#include "Logger.h"

pLayout buttonNext;
GLuint textureButton;

const char* filesName[] = {"Pictures/Calibration/buttonNext.png"};

/*!\brief Fonction qui initialise le Mode Calibration */
int initCalibrationMode()
{
    int statusValue;
    
    statusValue = InitTextures();
    buttonNext = (pLayout)malloc(sizeof(SLayout));
    assert(buttonNext);
    buttonNext->surface = gl4dgGenQuadf();
    setCalibrationModeLayoutDimensions();
    
    return statusValue;
}

/*!\brief Fonction qui affiche l'écran de calibration ainsi que ses layouts */
void CalibrationMode()
{
    static bool isMocapReady = false;
    
    if(isMocapReady == false)
        if(isCalibrationReady() == true)
            isMocapReady = true;
    
    if(isMocapReady)
    {
        printButtonNext();
        //printHands();
        if(isClickedButtonNext() == true)
        {
            freeCalibrationMode();
            MODE = MENU;
        }
    }
}

/*!\brief Fonction teste si le bouton "Next" a été cliqué ou non */
bool isClickedButtonNext()
{
    int delay = 4;
    int x,y;
    SDL_PumpEvents();
    SDL_GetMouseState(&x, &y);
    int buttonWidth  = buttonNext->width;
    int buttonHeight = buttonNext->height;
    int buttonX = buttonNext->x;
    int buttonY = buttonNext->y;
    
    if(MAIN_DROITE->isAvailable())
    {
        int mainX = MAIN_DROITE->x;
        int mainY = MAIN_DROITE->y;
        
        if(mainX >= (buttonX-buttonWidth) && mainX <= (buttonX + buttonWidth) &&
           mainY >= (buttonY-buttonHeight) && mainY <= (buttonY + buttonHeight))
        {
            return true;
        }
    }
}

/*!\brief Fonction qui initialise les dimensions du bouton en fonction des dimensions de la fenêtre
 Fonction rappelée lors du redimensionnement de la fenêtre */
void setCalibrationModeLayoutDimensions()
{
    if(buttonNext != NULL)
    {
        buttonNext->width   = _windowWidth/6;
        buttonNext->height  = _windowHeight/20;
        buttonNext->x       = _windowWidth - buttonNext->width - 10;
        buttonNext->y       = 50;
    }
}

/*!\brief Fonction Qui affiche le boutton Next */
void printButtonNext()
{
    //Draw Button NEXT
    gl4duLoadIdentityf();
    gl4duTranslatef(buttonNext->x, buttonNext->y, -11);
    gl4duScalef(buttonNext->width, buttonNext->height, 1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, textureButton);
    gl4dgDraw(buttonNext->surface);
}

/*!\brief Fonction qui charge les textures*/
int InitTextures()
{
    SDL_Surface *t;
    int i;
    
    glGenTextures(1, &textureButton);
    glEnable(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, textureButton);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if((t = IMG_Load(filesName[0])) != NULL )
    {
#ifdef __APPLE__
        int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
        int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif
            
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA , GL_UNSIGNED_BYTE, t->pixels);
            
        SDL_FreeSurface(t);
    }
    else
    {
        fprintf(stderr, "can't open file %s : %s\n", filesName[0], SDL_GetError());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        return -1;
    }
    
    return 0;
}

void freeCalibrationMode()
{
    glDeleteTextures(1, &textureButton);
}
