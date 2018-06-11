#include "Window.h"
#include "Logger.h"
#include "Mocap.h"
#include <GL4D/gl4du.h>
#include "Device.h"
#include "GL4D/gl4dp.h"
#include <SDL2/SDL_image.h>
#include "NiteSampleUtilities.h"
#include "string.h"

struct timespec startt, stopp;

void drawLineSkeleton();
int testPoseExit(nite::UserTrackerFrameRef* userTrackerFrame, const nite::UserData& user);
void printFrameKinect(nite::UserTrackerFrameRef* userTrackerFrame);
static Uint32 _cur_color = 0xFF0000FF;
#define NUMBER_TEXTURE        7

#define TEXTURE_HAND          0
#define TEXTURE_ARROW         1
#define TEXTURE_BACKGROUND    2

#define TEXTURE_SIZE          1
#define MIN_NUM_CHUNKS(data_size, chunk_size)    ((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)    (MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))
openni::RGB888Pixel*        m_pTexMap;
unsigned int m_nTexMapX, m_nTexMapY;

// time to hold in pose to exit program. In milliseconds.
const int g_poseTimeoutToExit = 2000;
nite::UserTracker* m_pUserTracker;
nite::UserId m_poseUser;
uint64_t m_poseTime;
GLuint _frameKinect = 0;
openni::VideoFrameRef depthFrame;
bool printBackground = true;

int currentAvatar = 0;

const char *FBXPath[] =         {
    "FBXLoader/Models/Zombie/Phychozomb.fbx",
    "FBXLoader/Models/Girl/Girl.fbx",
    "FBXLoader/Models/Man/Man.fbx",
    "FBXLoader/Models/Peggy/Peggy.fbx",
    "FBXLoader/Models/mari/BrettHughes.fbx"};

const char *FBXNameMembers[] =  {
    "FBXLoader/Models/Zombie/Phychozomb.members",
    "FBXLoader/Models/Girl/Girl.members",
    "FBXLoader/Models/Man/Man.members",
    "FBXLoader/Models/Peggy/Peggy.members",
    "FBXLoader/Models/mari/BrettHughes.members"};

int g_nXRes = 0, g_nYRes = 0;
float m_pDepthHist[10000];
float Colorss[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
int colorCountt = 3;
static GLuint _texturesId[NUMBER_TEXTURE]; //0:Main , 1:Flèche
GLuint _background = 0;
static GLuint _sphere = 0;

SceneContext *gSceneContext = NULL;
pLayout _layout[4];
#define NUMBER_OF_LAYOUT        4

#define PICTURE_MAIN_DROITE     0
#define PICTURE_MAIN_GAUCHE     1
#define PICTURE_FLECHE_NEXT     2
#define PICTURE_FLECHE_PREV     3

JointCoordinates*  TETE              = new JointCoordinates();
JointCoordinates*  NUQUE             = new JointCoordinates();
JointCoordinates*  EPAULE_DROITE     = new JointCoordinates();
JointCoordinates*  EPAULE_GAUCHE     = new JointCoordinates();
JointCoordinates*  COUDE_GAUCHE      = new JointCoordinates();
JointCoordinates*  COUDE_DROIT       = new JointCoordinates();
JointCoordinates*  MAIN_GAUCHE       = new JointCoordinates();
JointCoordinates*  MAIN_DROITE       = new JointCoordinates();
JointCoordinates*  TORSE             = new JointCoordinates();
JointCoordinates*  HANCHE_GAUCHE     = new JointCoordinates();
JointCoordinates*  HANCHE_DROITE     = new JointCoordinates();
JointCoordinates*  GENOU_DROIT       = new JointCoordinates();
JointCoordinates*  GENOU_GAUCHE      = new JointCoordinates();
JointCoordinates*  PIED_DROIT        = new JointCoordinates();
JointCoordinates*  PIED_GAUCHE       = new JointCoordinates();
JointCoordinates*  BASE_COLONNE_V    = new JointCoordinates();

JointCoordinates*  tempGenouGauche   = new JointCoordinates();
JointCoordinates*  tempGenouDroit    = new JointCoordinates();
JointCoordinates*  tempNuque         = new JointCoordinates();

/*!\brief Fonction qui initialise les layouts et les FBX du mode de Motion Capture */
int initAvatarMocap()
{
    for(int i=0 ; i<NUMBER_OF_LAYOUT ; i++)
    {
        _layout[i] = (pLayout)malloc(sizeof(SLayout));
        assert(_layout[i]);
        _layout[i]->surface = gl4dgGenQuadf();
    }
    _layout[PICTURE_MAIN_DROITE]->x = 0;
    _layout[PICTURE_MAIN_DROITE]->y = 0;
    _layout[PICTURE_MAIN_DROITE]->width = 15;
    _layout[PICTURE_MAIN_DROITE]->height = 20;
    
    _layout[PICTURE_MAIN_GAUCHE]->x = 0;
    _layout[PICTURE_MAIN_GAUCHE]->y = 0;
    _layout[PICTURE_MAIN_GAUCHE]->width = 15;
    _layout[PICTURE_MAIN_GAUCHE]->height = 20;
    
    _layout[PICTURE_FLECHE_NEXT]->x = _windowWidth - 100;
    _layout[PICTURE_FLECHE_NEXT]->y = _windowHeight / 2;
    _layout[PICTURE_FLECHE_NEXT]->width = 40;
    _layout[PICTURE_FLECHE_NEXT]->height = 40;
    
    _layout[PICTURE_FLECHE_PREV]->x = 100;
    _layout[PICTURE_FLECHE_PREV]->y = _windowHeight / 2;
    _layout[PICTURE_FLECHE_PREV]->width = 40;
    _layout[PICTURE_FLECHE_PREV]->height = 40;
    
    _sphere = gl4dgGenSpheref(30, 30);
    _background = gl4dgGenQuadf();
    _frameKinect = gl4dgGenQuadf();
    initTextures();
    
    if(CreateContext(FBXPath[currentAvatar]) != 0)
        return -1;
    
    if(readMembersName(FBXNameMembers[currentAvatar]) != 0)
        return -1;
    
    return 0;
}

/*!\brief Fonction qui initialise le FBX en lui créant un contexte indépendant */
int CreateContext(const char *file)
{
    if((gSceneContext = new SceneContext(file)) == NULL)
        return -1;
    
    return 0;
}

/*!\brief Fonction qui charge les tectures (background, flèches ...) */
static void initTextures()
{
    SDL_Surface *t;
    const char *files[] = {"Pictures/Mocap/glove.png",
                           "Pictures/Mocap/arrow.png",
                           "Pictures/Mocap/background1.jpg",
                           "Pictures/Mocap/background2.jpg",
                           "Pictures/Mocap/background3.jpg",
                           "Pictures/Mocap/background4.jpg",
                           "Pictures/Mocap/background5.jpg" };
    
    glGenTextures(NUMBER_TEXTURE, _texturesId);
    for(int i = 0; i<NUMBER_TEXTURE ; i++)
    {
        glBindTexture(GL_TEXTURE_2D, _texturesId[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        if((t = IMG_Load(files[i])) != NULL )
        {
            if(i >= 2)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGB, GL_UNSIGNED_BYTE, t->pixels);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
            SDL_FreeSurface(t);
        }
        else
        {
            fprintf(stderr, "can't open file %s : %s\n", files[i], SDL_GetError());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            exit(-1);
        }
    }
}

/*!\brief Fonction qui copie le nom d'un membre */
void copyMembersName(char **dst , char* src)
{
    int size = strlen(src);
    if(src[size-1] == '\n')
    {
        src[size-1] = '\0';
        size--;
    }
    
    *dst = (char*)malloc(size+1 * sizeof(char));
    assert(*dst);
    strcpy(*dst,src);
}

/*!\brief Fonction qui lie et récupère les informations du fichier d'articulations des FBX (fichier servant à connaitre les noms des articulations du FBX) */
int readMembersName(const char *filename)
{
    FILE *data = NULL;
    data = fopen(filename,"r");
    if(data == NULL)
    {
        fprintf(stderr,"Error : The members names file is not correct or not founded ! \n");
        return -1;
    }
    else
        LOG4CXX_INFO(GlobalLogger, "Noms des noeuds du squelette du FBX charger avec succes !");
    
    int i=0;
    char line[50];
    char *token;
    char delim[4] = "= ";
    
    while(fgets(line, sizeof(line), data) != NULL)
    {
        
        token = strtok(line, delim);
        token = strtok(NULL, delim);
        
        if(i==0)        { copyMembersName( &TETE->name           , token );        TETE->member_name            = "TETE";            }
        else if(i==1)   { copyMembersName( &NUQUE->name          , token );        NUQUE->member_name           = "NUQUE";           }
        else if(i==2)   { copyMembersName( &EPAULE_DROITE->name  , token );        EPAULE_DROITE->member_name   = "EPAULE_DROITE";   }
        else if(i==3)   { copyMembersName( &EPAULE_GAUCHE->name  , token );        EPAULE_GAUCHE->member_name   = "EPAULE_GAUCHE";   }
        else if(i==4)   { copyMembersName( &COUDE_DROIT->name    , token );        COUDE_DROIT->member_name     = "COUDE_DROIT";     }
        else if(i==5)   { copyMembersName( &COUDE_GAUCHE->name   , token );        COUDE_GAUCHE->member_name    = "COUDE_GAUCHE";    }
        else if(i==6)   { copyMembersName( &MAIN_DROITE->name    , token );        MAIN_DROITE->member_name     = "MAIN_DROITE";     }
        else if(i==7)   { copyMembersName( &MAIN_GAUCHE->name    , token );        MAIN_GAUCHE->member_name     = "MAIN_GAUCHE";     }
        else if(i==8)   { copyMembersName( &HANCHE_DROITE->name  , token );        HANCHE_DROITE->member_name   = "HANCHE_DROITE";   }
        else if(i==9)   { copyMembersName( &HANCHE_GAUCHE->name  , token );        HANCHE_GAUCHE->member_name   = "HANCHE_GAUCHE";   }
        else if(i==10)  { copyMembersName( &GENOU_DROIT->name    , token );        GENOU_DROIT->member_name     = "GENOU_DROIT";     }
        else if(i==11)  { copyMembersName( &GENOU_GAUCHE->name   , token );        GENOU_GAUCHE->member_name    = "GENOU_GAUCHE";    }
        else if(i==12)  { copyMembersName( &PIED_DROIT->name     , token );        PIED_DROIT->member_name      = "PIED_DROIT";      }
        else if(i==13)  { copyMembersName( &PIED_GAUCHE->name    , token );        PIED_GAUCHE->member_name     = "PIED_GAUCHE";     }
        else if(i==14)  { copyMembersName( &BASE_COLONNE_V->name , token );        BASE_COLONNE_V->member_name  = "BASE_COLONNE_V";  }
        else break;
        
        i++;
    }
    
    fclose(data);
    return 0;
}

/*!\brief Fonction qui récupère les frames de la kinect, et track les utilisateurs. Une fois qu'un utilisateurs est trouvé, la fonction de detection du squelette et
 des membres est appelée */
void trackUser()
{
    nite::UserTrackerFrameRef userTrackerFrame;
    nite::Status rc = m_pUserTracker->readFrame(&userTrackerFrame);
    if (rc != nite::STATUS_OK)
    {
        printf("GetNextData failed\n");
        return;
    }
    
    depthFrame = userTrackerFrame.getDepthFrame();
    g_nXRes = depthFrame.getVideoMode().getResolutionX();
    g_nYRes = depthFrame.getVideoMode().getResolutionY();

    if(MODE != GAME)
    printFrameKinect(&userTrackerFrame);
    
    const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();
    for (int i = 0; i < users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];
    
        if (user.isNew())
        {
            fprintf(stderr,"NEEEEEEWW\n");
            printBackground = false;
            m_pUserTracker->startSkeletonTracking(user.getId());
            m_pUserTracker->startPoseDetection(user.getId(), nite::POSE_CROSSED_HANDS);
        }
        else if (!user.isLost())
        {
            if (users[i].getSkeleton().getState() == nite::SKELETON_TRACKED)
            {
                //fprintf(stderr,"TRACKEDDD\n");
                DrawSkeleton(m_pUserTracker, user);
            }
        }
  
        static int locker = 0;
        if(locker == 0)
            locker = testPoseExit(&userTrackerFrame , user);
        
        if(locker == 1)
        {
            clock_gettime( CLOCK_REALTIME, &startt);
            locker=2;
        }
        
        clock_gettime( CLOCK_REALTIME, &stopp);
        if(stopp.tv_sec - startt.tv_sec >= 3)
            locker = 0;
    }
}

/*!\brief Fonction qui affiche les frames de la kinect en fonction du MODE définit :
 *
 *  - Mode CALIBRATION   : frame prend toute la dimensions de la fenêtre
 *  - Mode MENU et MOCAP : frame encadré dans un rectangle en haut a droite de la fenêtre
 *  - Mode GAME          : Pas de frame affichée
 */
void printFrameKinect(nite::UserTrackerFrameRef* userTrackerFrame)
{
    float Colors[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
    int colorCount = 3;
    GLfloat lumpos[4] = {0.0, 1.0, 1.0, 0.0};
    
    if (m_pTexMap == NULL)
    {
        // Texture map init
        m_nTexMapX = MIN_CHUNKS_SIZE(g_nXRes, TEXTURE_SIZE);
        m_nTexMapY = MIN_CHUNKS_SIZE(g_nYRes, TEXTURE_SIZE);
        m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
    }
    
    const nite::UserMap& userLabels = userTrackerFrame->getUserMap();
    const nite::UserId* pLabels = userLabels.getPixels();
    
    calculateHistogram(m_pDepthHist, 10000, depthFrame);
    memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));
    
    const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
    openni::RGB888Pixel* pTexRow = m_pTexMap + depthFrame.getCropOriginY() * m_nTexMapX;
    int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);
    
    float factor[3] = {1, 1, 1};
    for (int y = 0; y < depthFrame.getHeight(); ++y)
    {
        const openni::DepthPixel* pDepth = pDepthRow;
        openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();
        
        for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth, ++pTex, ++pLabels)
        {
            if (*pDepth != 0)
            {
                if (*pLabels == 0)
                {
                    if (!printBackground)
                    {
                        factor[0] = factor[1] = factor[2] = 0;
                        
                    }
                    else
                    {
                        factor[0] = Colors[colorCount][0];
                        factor[1] = Colors[colorCount][1];
                        factor[2] = Colors[colorCount][2];
                    }
                }
                else
                {
                    factor[0] = Colors[*pLabels % colorCount][0];
                    factor[1] = Colors[*pLabels % colorCount][1];
                    factor[2] = Colors[*pLabels % colorCount][2];
                }
                //                    // Add debug lines - every 10cm
                //                     else if ((*pDepth / 10) % 10 == 0)
                //                     {
                //                         factor[0] = factor[2] = 0;
                //                     }
                
                int nHistValue = m_pDepthHist[*pDepth];
                pTex->r = nHistValue*factor[0];
                pTex->g = nHistValue*factor[1];
                pTex->b = nHistValue*factor[2];
                
                factor[0] = factor[1] = factor[2] = 1;
            }
        }
        pDepthRow += rowSize;
        pTexRow += m_nTexMapX;
    }

    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);
    
    float _frameKinectW = _windowWidth/8;
    float _frameKinectH = _windowHeight/8;
    
    gl4duPushMatrix();
    gl4duLoadIdentityf();
    glBindTexture(GL_TEXTURE_2D, t);
    glUniform1i(glGetUniformLocation(_pIdFBX, "myTexture"), 0);
    glUniform4fv(glGetUniformLocation(_pIdFBX, "lumpos"), 1, lumpos);
    
    if(MODE == CALIBRATION)
    {
        gl4duTranslatef(_windowWidth/2, _windowHeight/2, -40);
        gl4duScalef(_windowWidth/2, _windowHeight/2, 1);
    }
    else
    {
        gl4duTranslatef(_windowWidth-_frameKinectW, _frameKinectH, -1);
        gl4duScalef(_frameKinectW, _frameKinectH, 1);
    }
    gl4duSendMatrices();
    gl4dgDraw(_frameKinect);
    gl4duPopMatrix();
}

/*!\brief Fonction qui analyse la position de l'utilisateur afin de détécter la position EXIT (croisement des bras en forme de X) et applique une action en focntion du MODE courant */
int testPoseExit(nite::UserTrackerFrameRef* userTrackerFrame, const nite::UserData& user)
{

    if (m_poseUser == 0 || m_poseUser == user.getId())
    {
        const nite::PoseData& pose = user.getPose(nite::POSE_CROSSED_HANDS);
        
        if (pose.isEntered())
        {
            // Start timer
            //sprintf(g_generalMessage, "In exit pose. Keep it for %d second%s to exit\n", g_poseTimeoutToExit/1000, g_poseTimeoutToExit/1000 == 1 ? "" : "s");
            fprintf(stderr,"Counting down %d second to exit\n", g_poseTimeoutToExit/1000);
            m_poseUser = user.getId();
            m_poseTime = userTrackerFrame->getTimestamp();
        }
        else if (pose.isExited())
        {
            //memset(g_generalMessage, 0, sizeof(g_generalMessage));
            fprintf(stderr,"Count-down interrupted\n");
            m_poseTime = 0;
            m_poseUser = 0;
        }
        else if (pose.isHeld())
        {
            // tick
            if (userTrackerFrame->getTimestamp() - m_poseTime > g_poseTimeoutToExit * 1000)
            {
                printf("Count down complete. Exit...\n");
                
                if(MODE == CALIBRATION || MODE == MOCAP || MODE == GAME)
                {
                    MODE = MENU;
                    m_poseTime = 0;
                    m_poseUser = 0;
                    adaptTheLandmark();
                    return 1;
                }
                
                else if(MODE == MENU)
                {
                    quit();
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
    
    return 0;
}

/*!\brief Fonction qui récupère la position de chaque articulations et définit son degré de validité :
 *
 *  - Si degré compris entre 0 et 0.5 le membre est considéré comme non fiable et non valide
 *  - Si degré supérieur à 0.5 le membre est considéré comme valide
 */
void GetMembersCoordinates(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, int color, JointCoordinates* member)
{
    float coordinates[3] = {0};
    
    pUserTracker->convertJointCoordinatesToDepth(joint1.getPosition().x, joint1.getPosition().y, joint1.getPosition().z, &coordinates[0], &coordinates[1]);
    
    coordinates[0] *= (_windowWidth)/(float)g_nXRes;
    coordinates[1] *= (_windowHeight)/(float)g_nYRes;
    //coordinates[2] *= (_windowHeight)/(float)g_nYRes;
    
    member->x = coordinates[0];
    member->y = coordinates[1];
    member->z = joint1.getPosition().z;
    
    //fprintf(stderr," x = %f   ,   y = %f   , RESX = %d \n",coordinates[0], coordinates[1], g_nXRes);
    if(joint1.getPositionConfidence() == 1)
    {
        //glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);
        member->available = true;
    }
    else if(joint1.getPositionConfidence() > 0.5f)
    {
        //glColor3f(1.0f - Colors[color][0], 1.0f - Colors[color][1], 1.0f - Colors[color][2]);
        //member->available = true;
    }
    else if (joint1.getPositionConfidence() < 0.5f)
    {
        member->available = false;
        return;
    }
    else
    {
        glColor3f(0, 1, 0);
    }

    if(MODE == CALIBRATION)
    {
        GLfloat rouge[4] = {1,0,0,1};
        gl4duLoadIdentityf();
        glUniform1i(glGetUniformLocation(_pIdFBX, "colorTexture"), 1);
        glUniform4fv(glGetUniformLocation(_pIdFBX, "couleur"), 1, rouge);
        gl4duTranslatef(member->x, member->y, -10);
        gl4duScalef(10,10,1);
        gl4duSendMatrices();
        if(member->available == true)
            gl4dgDraw(_sphere);
        glUniform1i(glGetUniformLocation(_pIdFBX, "colorTexture"), 0);
    }
}

/*!\brief Fonction qui récupère les coordonnées de chaque membres */
void DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData)
{
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_HEAD)           , userData.getId() % colorCountt , TETE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_NECK)           , userData.getId() % colorCountt , NUQUE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_SHOULDER) , userData.getId() % colorCountt , EPAULE_DROITE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_SHOULDER)  , userData.getId() % colorCountt , EPAULE_GAUCHE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_ELBOW)    , userData.getId() % colorCountt , COUDE_DROIT);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_ELBOW)     , userData.getId() % colorCountt , COUDE_GAUCHE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND)     , userData.getId() % colorCountt , MAIN_DROITE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_HAND)      , userData.getId() % colorCountt , MAIN_GAUCHE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_TORSO)          , userData.getId() % colorCountt , TORSE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_HIP)      , userData.getId() % colorCountt , HANCHE_DROITE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_HIP)       , userData.getId() % colorCountt , HANCHE_GAUCHE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_KNEE)     , userData.getId() % colorCountt , GENOU_DROIT);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_KNEE)      , userData.getId() % colorCountt , GENOU_GAUCHE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_RIGHT_FOOT)     , userData.getId() % colorCountt , PIED_DROIT);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_FOOT)      , userData.getId() % colorCountt , PIED_GAUCHE);
    GetMembersCoordinates(pUserTracker, userData.getSkeleton().getJoint(nite::JOINT_LEFT_FOOT)      , userData.getId() % colorCountt , PIED_GAUCHE);
}

/*!\brief Fonction qui retourne TRUE si tous les membres du squelettes ont bien été détécté (degré de validité > 0.5)
 et False dansle cas contraire */
bool isCalibrationReady()
{
    int acc = 0;
    
    if(TETE->available)             acc++;
    if(NUQUE->available)            acc++;
    if(EPAULE_DROITE->available)    acc++;
    if(EPAULE_GAUCHE->available)    acc++;
    if(COUDE_DROIT->available)      acc++;
    if(COUDE_GAUCHE->available)     acc++;
    if(MAIN_DROITE->available)      acc++;
    if(MAIN_GAUCHE->available)      acc++;
    if(HANCHE_DROITE->available)    acc++;
    if(HANCHE_GAUCHE->available)    acc++;
    if(GENOU_DROIT->available)      acc++;
    if(GENOU_GAUCHE->available)     acc++;
    if(PIED_DROIT->available)       acc++;
    if(PIED_GAUCHE->available)      acc++;
    if(BASE_COLONNE_V->available)   acc++;
    
    if(acc >= 15)
        return true;
    
    return false;
}

/*!\brief Fonction qui affiche les deux flèches permettant de passer au FBX suivant ou précédent*/
static void printArrows()
{
    //Flèche droite (Modèle suivant)
    gl4duPushMatrix();
    gl4duLoadIdentityf();
    _layout[PICTURE_FLECHE_NEXT]->x = _windowWidth - 100;
    _layout[PICTURE_FLECHE_NEXT]->y = _windowHeight / 2;
    glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_ARROW]);
    gl4duTranslatef(_layout[PICTURE_FLECHE_NEXT]->x, _layout[PICTURE_FLECHE_NEXT]->y, -2);
    gl4duScalef(_layout[PICTURE_FLECHE_NEXT]->width, _layout[PICTURE_FLECHE_NEXT]->height, 0);
    gl4duSendMatrices();
    gl4dgDraw(_layout[PICTURE_FLECHE_NEXT]->surface);
    
    //Flèche gauche (Modèle précédent)
    gl4duLoadIdentityf();
    _layout[PICTURE_FLECHE_PREV]->x = 100;
    _layout[PICTURE_FLECHE_PREV]->y = _windowHeight / 2;
    glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_ARROW]);
    gl4duTranslatef(_layout[PICTURE_FLECHE_PREV]->x, _layout[PICTURE_FLECHE_PREV]->y, -2);
    gl4duScalef(_layout[PICTURE_FLECHE_PREV]->width, _layout[PICTURE_FLECHE_PREV]->height, 0);
    gl4duRotatef(180,0,1,0);
    gl4duSendMatrices();
    gl4dgDraw(_layout[PICTURE_FLECHE_PREV]->surface);
    gl4duPopMatrix();
}

/*!\brief Fonction qui affiche les mains, permettant de cliquer sur les flèches*/
void printHands()
{
    //glUniform1i(glGetUniformLocation(_pIdFBX, "myTexture"),0);
    //Main droite
    glEnable(GL_BLEND);
    if(MAIN_DROITE->available)
    {
        gl4duPushMatrix();
        _layout[PICTURE_MAIN_DROITE]->x = MAIN_DROITE->x;
        _layout[PICTURE_MAIN_DROITE]->y = MAIN_DROITE->y;
        gl4duLoadIdentityf();
        glBindTexture(GL_TEXTURE_2D, _texturesId[TEXTURE_HAND]);
        gl4duTranslatef( _layout[PICTURE_MAIN_DROITE]->x,  _layout[PICTURE_MAIN_DROITE]->y, -1);
        gl4duScalef( _layout[PICTURE_MAIN_DROITE]->width,  _layout[PICTURE_MAIN_DROITE]->height, 0);
        gl4duRotatef(-COUDE_DROIT->angle - 90,0,0,1);
        gl4duSendMatrices();
        gl4dgDraw(_layout[PICTURE_MAIN_DROITE]->surface);
        gl4duPopMatrix();
    }
    
    //Main Gauche
    if(MAIN_GAUCHE->available)
    {
        gl4duPushMatrix();
        _layout[PICTURE_MAIN_GAUCHE]->x = MAIN_GAUCHE->x;
        _layout[PICTURE_MAIN_GAUCHE]->y = MAIN_GAUCHE->y;
        gl4duLoadIdentityf();
        glBindTexture(GL_TEXTURE_2D, _texturesId[0]);
        gl4duTranslatef( _layout[PICTURE_MAIN_GAUCHE]->x,  _layout[PICTURE_MAIN_GAUCHE]->y, -1);
        gl4duScalef( _layout[PICTURE_MAIN_GAUCHE]->width,  _layout[PICTURE_MAIN_GAUCHE]->height, 0);
        gl4duRotatef(180,0,1,0);
        gl4duRotatef(-COUDE_GAUCHE->angle-90,0,0,1);
        gl4duSendMatrices();
        gl4dgDraw(_layout[PICTURE_MAIN_GAUCHE]->surface);
        gl4duPopMatrix();
    }
}


static struct timespec start, stop;
/*!\brief Fonction qui analyse si la flèche permettant de passer au FBX suivant été cliquée ou non */
static bool isNextClicked()
{
    static bool isHoover = false;
    int delay = 2;
    
    if(MAIN_DROITE->isAvailable())
    {
        int mainX = MAIN_DROITE->x;    int flecheX = _layout[PICTURE_FLECHE_NEXT]->x;
        int mainY = MAIN_DROITE->y;    int flecheY = _layout[PICTURE_FLECHE_NEXT]->y;
        int flecheWidth  = _layout[PICTURE_FLECHE_NEXT]->width;
        int flecheHeight = _layout[PICTURE_FLECHE_NEXT]->height;
        
        if(mainX >= (flecheX-flecheWidth) && mainX <= (flecheX + flecheWidth) &&
           mainY >= (flecheY-flecheHeight) && mainY <= (flecheY + flecheHeight))
        {
            //Enregistrement du temp de depart
            if(isHoover == false)
            {
                isHoover = true;
                clock_gettime( CLOCK_REALTIME, &start);
            }
            
            //zoomLayout(true, buttonId);
            
            clock_gettime( CLOCK_REALTIME, &stop);
            float timeElapsed = stop.tv_sec - start.tv_sec;
            
            //Si le temp d'arrivée - temp de depart >= delai alors return true
            if(timeElapsed >= delay)
            {
                isHoover = false;
                return true;
            }
            
            LOG4CXX_INFO(GlobalLogger, "[NEXT] Chargement du modele suivant ! ");
            return false;
        }
    }
    
    isHoover = false;
    return false;
}

/*!\brief Fonction qui analyse si la flèche permettant de passer au FBX précédent à été cliquée ou non */
static bool isPreviousClicked()
{
    static bool isHoover = false;
    int delay = 2;
    
    if(MAIN_GAUCHE->isAvailable())
    {
        int mainX = MAIN_GAUCHE->x;    int flecheX = _layout[PICTURE_FLECHE_PREV]->x;
        int mainY = MAIN_GAUCHE->y;    int flecheY = _layout[PICTURE_FLECHE_PREV]->y;
        int flecheWidth  = _layout[PICTURE_FLECHE_PREV]->width;
        int flecheHeight = _layout[PICTURE_FLECHE_PREV]->height;
        
        if(mainX >= (flecheX-flecheWidth) && mainX <= (flecheX + flecheWidth) &&
           mainY >= (flecheY-flecheHeight) && mainY <= (flecheY + flecheHeight))
        {
            //Enregistrement du temp de depart
            if(isHoover == false)
            {
                isHoover = true;
                clock_gettime( CLOCK_REALTIME, &start);
            }
            
            //zoomLayout(true, buttonId);
            
            clock_gettime( CLOCK_REALTIME, &stop);
            float timeElapsed = stop.tv_sec - start.tv_sec;
            
            //Si le temp d'arrivée - temp de depart >= delai alors return true
            if(timeElapsed >= delay)
            {
                isHoover = false;
                return true;
            }
            
            LOG4CXX_INFO(GlobalLogger, "[PREV] Chargement du modele precedent ! ");
            return false;
        }
    }
    
    return false;
}

/*!\brief Fonction qui permet de dessiner le FBX */
static void drawAvatar()
{
    //static bool isSkeletonAvailable = isCalibrationReady();
   
    //if(isSkeletonAvailable == false)
        //return;

    gl4duPushMatrix();
    GLfloat lumpos[4] = {0.0, 1.0, 1.0, 0.0};
    gl4duLoadIdentityf();
    
    gl4duTranslatef(_windowWidth/2, _windowHeight/1.1, -80);

    gl4duScalef(3,3,3);
    //gl4duRotatef(90,1,0,0);
    gl4duRotatef(180,0,0,1);
    gl4duSendMatrices();
    glUniform1i(glGetUniformLocation(_pIdFBX, "myTexture"),0);
    glUniform4fv(glGetUniformLocation(_pIdFBX, "lumpos"), 1, lumpos);
    //if(currentAvatar == 0)
        //glUniform1i(glGetUniformLocation(_pIdFBX, "lightFBX"), 1);
    gSceneContext->OnDisplay();
    
    //Importer la scène
    if (gSceneContext->GetStatus() == SceneContext::MUST_BE_LOADED)
    {
        gSceneContext->LoadFile();
        //gSceneContext->SetCurrentAnimStack(0);
    }
    gl4duPopMatrix();
}

/*!\brief Fonction qui  calcule l'angle de rotation de chaque membre en fonction de ses dépendances
 (La fonction de rotation des membres se trouve à l'emplacement /FBXLoader/DrawScene.cpp */
void calculateMembersAngle()
{
    EPAULE_DROITE->calculateAngle  ( COUDE_DROIT   , NUQUE );
    EPAULE_GAUCHE->calculateAngle  ( COUDE_GAUCHE  , NUQUE );
    COUDE_DROIT->calculateAngle    ( MAIN_DROITE   , EPAULE_DROITE );
    COUDE_GAUCHE->calculateAngle   ( MAIN_GAUCHE   , EPAULE_GAUCHE );
    GENOU_DROIT->calculateAngle    ( HANCHE_DROITE , PIED_DROIT    );
    GENOU_GAUCHE->calculateAngle   ( HANCHE_GAUCHE , PIED_GAUCHE   );
    
    //Colonne vertébrale, représente par le milieu des hanches
    if(HANCHE_GAUCHE->isAvailable() && HANCHE_DROITE->isAvailable())
    {
        BASE_COLONNE_V->x = HANCHE_GAUCHE->x + ((HANCHE_DROITE->x - HANCHE_GAUCHE->x) / 2);
        BASE_COLONNE_V->y = HANCHE_GAUCHE->y + ((HANCHE_DROITE->y - HANCHE_GAUCHE->y) / 2);
        BASE_COLONNE_V->available = true;
        tempNuque->x = BASE_COLONNE_V->x;
        tempNuque->y = BASE_COLONNE_V->y - 10; // 10 est définit au hasard, il fautr juste que ce point soit plus haut que le point de l'angle a calculer
        tempNuque->available = true;
        BASE_COLONNE_V->calculateAngle ( NUQUE , tempNuque );
    }
    
    //Jambes
    tempGenouDroit->x = HANCHE_DROITE->x;
    tempGenouDroit->y = HANCHE_DROITE->y + 10; // 10 est définit au hasard, il faut que ce poinnt soit plus bas que le point de l'angle a calculer
    tempGenouDroit->available = true;
    HANCHE_DROITE->calculateAngle  ( GENOU_DROIT   , tempGenouDroit );
    
    tempGenouGauche->x = HANCHE_GAUCHE->x;
    tempGenouGauche->y = HANCHE_GAUCHE->y + 10;
    tempGenouGauche->available = true;
    HANCHE_GAUCHE->calculateAngle  ( GENOU_GAUCHE  , tempGenouGauche);
}

/*!\brief Fonction qui dessine l'arrière plan pour chaque FBX */
void drawBackground()
{
    gl4duLoadIdentityf();
    gl4duTranslatef(_windowWidth/2,_windowHeight/2, -100);
    gl4duScalef(_windowWidth/2, _windowHeight/2, 1);
    gl4duSendMatrices();
    glBindTexture(GL_TEXTURE_2D, _texturesId[currentAvatar % NUMBER_FBX+2]);
    gl4dgDraw(_background);
}

/*!\brief Fonction d'affichage qui, dessine tous le contenu du mode MOTIONCAPTURE */
void drawMocap()
{
    drawBackground();
    drawAvatar();
    glEnable(GL_BLEND);
    printHands();
    printArrows();
    glDisable(GL_BLEND);
    
    if(isNextClicked())
    {
        delete gSceneContext;
        CreateContext(FBXPath[++currentAvatar % NUMBER_FBX]);
        readMembersName(FBXNameMembers[currentAvatar % NUMBER_FBX]);
    }
    if(isPreviousClicked())
    {
        delete gSceneContext;
        if(--currentAvatar < 0)
            currentAvatar = NUMBER_FBX - 1;
        CreateContext(FBXPath[currentAvatar % NUMBER_FBX]);
        readMembersName(FBXNameMembers[currentAvatar % NUMBER_FBX]);
    }
}

/*!\brief Fonction qui libère la mémoire du mode MotionCapture */
void freeMocap()
{
    LOG4CXX_INFO(GlobalLogger, "Liberation du mode Motion Capture !");
    delete gSceneContext;
    delete m_pUserTracker;
    
    if(TETE)            delete(TETE);
    if(NUQUE)           delete(NUQUE);
    if(EPAULE_DROITE)   delete(EPAULE_DROITE);
    if(EPAULE_GAUCHE)   delete(EPAULE_GAUCHE);
    if(COUDE_GAUCHE)    delete(COUDE_GAUCHE);
    if(COUDE_DROIT)     delete(COUDE_DROIT);
    if(TORSE)           delete(TORSE);
    if(MAIN_GAUCHE)     delete(MAIN_GAUCHE);
    if(MAIN_DROITE)     delete(MAIN_DROITE);
    if(HANCHE_GAUCHE)   delete(HANCHE_GAUCHE);
    if(HANCHE_DROITE)   delete(HANCHE_DROITE);
    if(GENOU_DROIT)     delete(GENOU_DROIT);
    if(GENOU_GAUCHE)    delete(GENOU_GAUCHE);
    if(PIED_DROIT)      delete(PIED_DROIT);
    if(PIED_GAUCHE)     delete(PIED_GAUCHE);
    
    for(int i=0 ; i<2 ; i++)
        glDeleteTextures(1, &_texturesId[i]);
    
    for(int i=0 ; i<NUMBER_OF_LAYOUT ; i++)
        if(_layout[i])
            free(_layout[i]);
}

