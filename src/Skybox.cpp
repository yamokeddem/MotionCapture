
#include <GL4D/gl4du.h>
#include <SDL2/SDL_image.h>
#include "Skybox.h"
#include "Logger.h"
#include "Window.h"

/*!\brief identifiant du (futur) vertex array object */
static GLuint _vao = 0;
/*!\brief identifiant des vertex array objects */
static GLuint skyboxVao[6] = {0};
/*!\brief identifiant des buffers de data */
static GLuint skyboxBuffer[6] = {0};
static GLuint skyboxId[6] = {0};

GLfloat _plan_r;
static GLfloat hauteur;
static GLfloat descente = hauteur;
static GLfloat _tex_repeat = 800;


/*!\brief Fonction qui charge les textures de la Skybox */
int loadPicturesSkybox()
{
    int i;
    SDL_Surface *t;

     const char * files[] = {   "Pictures/Skybox/avant.bmp",
     "Pictures/Skybox/arriere.bmp",
     "Pictures/Skybox/droite.bmp",
     "Pictures/Skybox/gauche.bmp",
     "Pictures/Skybox/grass.jpg",
     "Pictures/Skybox/haut.bmp"};
    
    glGenTextures(6, skyboxId);
    glEnable(GL_TEXTURE_2D);
    for(i = 0; i < 6; i++)
    {
        glBindTexture(GL_TEXTURE_2D, skyboxId[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if( (t = IMG_Load(files[i])) != NULL )
        {
#ifdef __APPLE__
            int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
            int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif
            
            if(i!=4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, mode, GL_UNSIGNED_BYTE, t->pixels);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , t->w, t->h, 0, GL_RGB , GL_UNSIGNED_BYTE, t->pixels);
            
            SDL_FreeSurface(t);
        }
        else
        {
            fprintf(stderr, "can't open file %s : %s\n", files[i], SDL_GetError());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            return -1;
        }
    }
}

/*!\brief Fonction qui initialise chaques faces de la Skybox */
int initSkybox()
{
    _plan_r = 500;
    hauteur =  300;
    descente =  hauteur;
    GLfloat s = _plan_r,
    
    data_skybox[6][20] =
    
    {
        //avant
        {
            -s , -hauteur,  -s,
            -s, hauteur,  -s,
            s, -hauteur,  -s,
            s , hauteur,  -s,
            1,1,
            1,0,
            0,1,
            0,0
        },
        //arriere
        {
            s, -descente, s,
            s , hauteur, s,
            -s, -descente, s,
            -s , hauteur, s,
            1,1,
            1,0,
            0,1,
            0,0
        },
        // droite
        {
            s, -descente, -s,
            s , hauteur, -s,
            s, -descente, s,
            s , hauteur, s,
            1,1,
            1,0,
            0,1,
            0,0
        },
        //gauche
        {
            -s, -descente, s,
            -s , hauteur, s,
            -s, -descente , -s,
            -s , hauteur, -s,
            1,1,
            1,0,
            0,1,
            0,0
        },
        //sol
        {
            -s, 0, s,
            -s , 0, -s,
            s, 0,  s,
            s , 0,  -s,
            _tex_repeat, _tex_repeat,
            _tex_repeat , 0,
            0 ,_tex_repeat,
            0 , 0
        },
        //plafond
        {
            -s, hauteur, -s,
            -s , hauteur, s,
            s, hauteur,  -s,
            s , hauteur,  s,
            0,0,
            0,1,
            1,0,
            1,1
        }
    };
    
    if(loadPicturesSkybox() == -1)
        return -1;
    
    // TEXTURE SKYBOX-----------------
    /*
     0 = avant
     1 = arriere
     2 = droite
     3 = gauche
     4 = sol
     5 = plafond
     */
    
    // VAO SKYBOX
    for(int i=0 ; i<6 ; i++)
    {
        glGenVertexArrays(1, &skyboxVao[i]);
        glBindVertexArray(skyboxVao[i]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(2);
        glGenBuffers(1, &skyboxBuffer[i]);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof data_skybox[i], data_skybox[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE,  0, (const void *)(4 * 3 * sizeof *data_skybox[i]));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        if(!skyboxVao[i])
            return -1;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    LOG4CXX_INFO(GlobalLogger, "Initialisation de la Skybox reussie !");
    
    return 0;
}

void drawSkybox()
{
    int i;
    glActiveTexture(GL_TEXTURE0);
    
    for(i=0 ; i<6 ; i++)
    {
        gl4duPushMatrix();
        glBindTexture(GL_TEXTURE_2D, skyboxId[i]);
        glBindVertexArray(skyboxVao[i]);
        gl4duSendMatrices();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        gl4duPopMatrix();
    }
    glBindVertexArray(0);
}

void freeSkybox()
{
    int i;
    //Free Textures
    for(i = 0;i<6 ; i++)
        if(skyboxId[i])
            glDeleteTextures(1, &skyboxId[i]);
    //FREE VAO
    for(i=0; i<6 ; i++)
        if(skyboxVao[i])
            glDeleteVertexArrays(1, &skyboxVao[i]);
    //Free Buffers
    for(i=0; i<6 ; i++)
        if(skyboxBuffer[i])
            glDeleteBuffers(1, &skyboxBuffer[i]);
}
