
#ifndef MOCAP_H
#define MOCAP_H
#include "NiTE.h"
#include "JointCoordinates.h"
#include "SceneContext.h"

#define NUMBER_FBX      5

extern int initAvatarMocap();
extern void printHands();
static void initTextures();
void printHands();
int CreateContext(const char *file);
static void printArrows();
int readMembersName(const char *filename);
static bool isPreviousClicked();
static bool isNextClicked();
void DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData);
void GetMembersCoordinates(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, int color, JointCoordinates* member);

extern nite::UserTracker* m_pUserTracker;
extern SceneContext * gSceneContext;
extern const char *FBXPath[] ;
extern const char *FBXNameMembers[];
extern int currentAvatar;
extern bool isCalibrationReady();
extern void drawMocap();
extern void freeMocap();
extern void calculateMembersAngle();
extern void trackUser();

#endif
