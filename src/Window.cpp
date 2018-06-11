#include "Window.h"
#include <GL4D/gl4duw_SDL2.h>
#include "NiTE.h"
#include <stdio.h>
#include "SceneContext.h"
#include "JointCoordinates.h"
#include "Logger.h"
#include "Skybox.h"
#include "Window.h"
#include <SDL2/SDL_image.h>
#include "Game.h"
#include "Menu.h"
#include "Device.h"
#include "Mocap.h"
#include "SoundEffect.h"
#include "CalibrationMode.h"
#include "CppUnitTest.h"

int DEBUG = 0;

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
# include <GL/freeglut.h>
#endif

static int initWindow(int argc, char **argv);
static void initOpenGL();
void initLog4CXX();
static void resize(int w, int h);
static void draw(void);
static void keydown(int keycode);


//Mode de début du programme (CALIBRATION afin que le programme se lance
//directement sur le mode de calibration de l'utilisateur)
int MODE = CALIBRATION;
cam_t _cam = {0, 50, 0};

//static int _windowWidth = 1280, _windowHeight = 1024;
int _windowWidth = 800;
int _windowHeight = 600;
GLuint _pIdFBX = 0;
LoggerPtr GlobalLogger;

/*!\brief Fonction Main du programme */
int main(int argc, char ** argv)
{
    CPPunitTest *CPPunit = new CPPunitTest();
    initLog4CXX();
    CPPunit->Test_ReturnValue (initWindow(argc, argv));
    
    if(!DEBUG)
        CPPunit->Test_DeviceInit (initNiteAndOpenNi());    // Chargement de la kinect, Nite et OpenNi
    initOpenGL();              // Chargement de OpenGL
    CPPunit->Test_ReturnValue (initSoundEffect());         // Chargement des effets sonores
    CPPunit->Test_ReturnValue (initCalibrationMode());     // Chargement du mode de calibration
    CPPunit->Test_ReturnValue (initSkybox());              // Chargement de la Skybox
    CPPunit->Test_ReturnValue (initAvatarMocap());         // Chargement du mode Motion Capture
    CPPunit->Test_ReturnValue (initMenu());                // Chargement du Menu
    CPPunit->Test_ReturnValue (initGame());                // Chargement du mode Jeu

    atexit(quit);
    gl4duwResizeFunc(resize);
    gl4duwKeyDownFunc(keydown);
    gl4duwDisplayFunc(draw);
    gl4duwMainLoop();
    
    return 0;
}

/*!\brief Fonction qui initialise la fenêtre */
static int initWindow(int argc, char **argv)
{
    fprintf(stderr,"\n\n");
    LOG4CXX_INFO(GlobalLogger, "------- CHARGEMENT DU PROGRAMMEEEEE ---------");
    
    if(!gl4duwCreateWindow(argc, argv, "Motion Capture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                           _windowWidth, _windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN))
    {
        fprintf(stdout,"\n\n");
        LOG4CXX_FATAL(GlobalLogger, "Initialisation de la fenetre echouee");
        return -1;
    }
    
    return 0;
}

/*!\brief Fonction qui initialise les paramètres OpenGL */
static void initOpenGL()
{
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.4f, 1.0f, 0.7f, 0.0f);
    _pIdFBX = gl4duCreateProgram("<vs>Shaders/fbx.vs", "<gs>Shaders/fbx.gs", "<fs>Shaders/fbx.fs", NULL);
    gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
    gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
    resize(_windowWidth, _windowHeight);
    
    fprintf(stdout,"\n");
    LOG4CXX_INFO(GlobalLogger, "Librairies OpenGL initialisees avec succes !");
}

/*!\brief Fonction qui initialise le framework Log4CXX */
void initLog4CXX()
{
    GlobalLogger = Logger::getLogger("Initialisation");

    FileAppender * fileAppender = new FileAppender(LayoutPtr(new SimpleLayout()), "Logs/Logs.txt", false);
    ConsoleAppender * consoleAppender = new ConsoleAppender(LayoutPtr(new SimpleLayout()));
    
    helpers::Pool p;
    fileAppender->activateOptions(p);
    
    BasicConfigurator::configure(AppenderPtr(fileAppender));
    BasicConfigurator::configure(AppenderPtr(consoleAppender));
    Logger::getRootLogger()->setLevel(Level::getDebug());
    
    fprintf(stdout,"\n\n");
    LOG4CXX_INFO(GlobalLogger, "Demarrage du Programme ! ");
    LOG4CXX_INFO(GlobalLogger, "Fichier de log cree avec succes ! ");
    
}

/*!\brief Fonction qui gère les évenements clavier */
static void keydown(int keycode)
{
    GLint v[2];
    
    switch(keycode)
    {
        case SDLK_ESCAPE:
        case 'q':
            exit(0);

            break;
        case SDLK_RIGHT:
            if(MODE == MOCAP)
            {
                delete gSceneContext;
                CreateContext(FBXPath[++currentAvatar % NUMBER_FBX]);
                readMembersName(FBXNameMembers[currentAvatar % NUMBER_FBX]);
                LOG4CXX_INFO(GlobalLogger, "[NEXT] Chargement du modele suivant ! ");
            }
            break;
        case SDLK_LEFT:
             if(MODE == MOCAP)
            {
                delete gSceneContext;
                if(--currentAvatar < 0)
                    currentAvatar = NUMBER_FBX - 1;
                CreateContext(FBXPath[currentAvatar % NUMBER_FBX]);
                fprintf(stderr,"modu = %d , numb = %d \n", (unsigned int)currentAvatar % NUMBER_FBX, NUMBER_FBX);
                readMembersName(FBXNameMembers[currentAvatar % NUMBER_FBX]);
                LOG4CXX_INFO(GlobalLogger, "[PREV] Chargement du modele precedent ! ");
            }
            break;

        case 'w':
            glGetIntegerv(GL_POLYGON_MODE, v);
            if(v[0] == GL_FILL) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.0);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glLineWidth(1.0);
            }
            break;
    
        default:
            break;
    }
}

/*!\brief Fonction qui permet d'adapter le repère en fonction du MODE choisis
*
*  - MODE CALIBRATION, MENU, ET MOCAP : repère orthogonale
*  - MODE GAME : repère Frustum
*/
void adaptTheLandmark()
{
    gl4duBindMatrix("projectionMatrix");
    gl4duLoadIdentityf();
    
    if(MODE != GAME)
        gl4duOrthof(0, _windowWidth,  _windowHeight, 0, 1, 1000.0);
    else
        gl4duFrustumf(-0.5, 0.5, -0.5 * _windowHeight / _windowWidth, 0.5 * _windowHeight / _windowWidth, 1.0, 100500.0);
    
      gl4duBindMatrix("modelViewMatrix");
}

/*!\brief Cette fonction parametre la vue (viewport) OpenGL en
 * fonction des dimensions de la fenêtre */
static void resize(int w, int h)
{
    _windowWidth  = w; _windowHeight = h;
    glViewport(0, 0, w, h);
    //initSkybox();
    setMenuLayoutDimensions();
    setCalibrationModeLayoutDimensions();
    adaptTheLandmark();
}

struct vertex
{
    GLfloat x, y, z;
};
typedef struct vertex vertex;

/*!\brief dessine dans le contexte OpenGL actif. */
static void draw(void)
{
    GLfloat lumpos[4] = {0.0, 1.0, 1.0, 0.0};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl4duBindMatrix("modelViewMatrix");
    gl4duLoadIdentityf();
    glUseProgram(_pIdFBX);

    if(!DEBUG)
    {
        trackUser();
        calculateMembersAngle();
    }
    
    if(MODE == CALIBRATION)
    {
        CalibrationMode();
    }
    
    if(MODE == MENU)
    {
        drawMenu();
        printHands();
    }
    
    if(MODE == MOCAP)
    {
        drawMocap();
    }
    
    if(MODE == GAME)
    {
        gl4duLookAtf(_cam.x  , 1.0, _cam.z ,
                     _cam.x  - sin(_cam.theta), 1.0 - (_windowHeight / 2 - (_windowHeight >> 1)) / (GLfloat)_windowHeight, _cam.z - cos(_cam.theta) ,
                     0.0, 1.0, 0.0);
        
        //Calcul de l'orientation et vitesse de l'avion
        setOrientationPlane();
        
        //Affichage des éléments du jeux
        drawGame();
    }
}

/*!\brief appelée au moment de sortir du programme (atexit), libère les éléments utilisés */
void quit(void)
{
    fprintf(stderr,"\n");
    LOG4CXX_INFO(GlobalLogger, "------- FIN DU PROGRAMMEEEEE ---------");
    gl4duClean(GL4DU_ALL);
    freeDevice();
    freeSkybox();
    freeMenu();
    freeGame();
    freeMocap();
    freeCalibrationMode();
    LOG4CXX_INFO(GlobalLogger, "Destruction du Logger !");
    LOG4CXX_INFO(GlobalLogger, "Fermeture de l'application !");
    GlobalLogger = 0;
}
