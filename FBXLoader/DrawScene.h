#ifndef _DRAW_SCENE_H
#define _DRAW_SCENE_H


#if defined(__APPLE__) || defined(MACOSX)                                                         
#include <OpenGL/gl3.h>
#else
#define  GL_GLEXT_PROTOTYPES
# include <GL/gl.h>
#endif

#include <fbxsdk.h>

void DrawNodeRecursive(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, 
                       FbxAMatrix& pParentGlobalPosition, FbxPose* pPose);

#endif // #ifndef _DRAW_SCENE_H






