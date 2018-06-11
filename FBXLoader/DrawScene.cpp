
#include "DrawScene.h"
#include "SceneCache.h"
#include "GetPosition.h"
#include "fbxsdk/scene/geometry/fbxnode.h"
#include "../src/JointCoordinates.h"
#include <string.h>
#include "GL4D/gl4du.h"
#include "../src/Mocap.h"

#ifdef DEBUG
#define INFO if(1) printf
#else 
#define INFO if(0) printf
#endif

void DrawNode(FbxNode* pNode, 
              FbxTime& lTime, 
              FbxAnimLayer * pAnimLayer,
              FbxAMatrix& pParentGlobalPosition,
              FbxAMatrix& pGlobalPosition,
              FbxPose* pPose);
void DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
              FbxAMatrix& pGlobalPosition, FbxPose* pPose);
void ComputeShapeDeformation(FbxMesh* pMesh, 
                             FbxTime& pTime, 
                             FbxAnimLayer * pAnimLayer,
                             FbxVector4* pVertexArray);
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, 
			       FbxMesh* pMesh,
			       FbxCluster* pCluster, 
			       FbxAMatrix& pVertexTransformMatrix,
			       FbxTime pTime, 
			       FbxPose* pPose);
void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, 
			      FbxMesh* pMesh, 
			      FbxTime& pTime, 
			      FbxVector4* pVertexArray,
			      FbxPose* pPose);
void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, 
				      FbxMesh* pMesh, 
				      FbxTime& pTime, 
				      FbxVector4* pVertexArray,
				      FbxPose* pPose);
void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, 
			    FbxMesh* pMesh, 
			    FbxTime& pTime, 
			    FbxVector4* pVertexArray,
			    FbxPose* pPose);
void ReadVertexCacheData(FbxMesh* pMesh, 
                         FbxTime& pTime, 
                         FbxVector4* pVertexArray);
void MatrixScale(FbxAMatrix& pMatrix, double pValue);
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue);
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix);


/*!\brief Fonction qui ajuste la sensibilité du modèle Fbx en testant si l'angle de rotation vaut la peine d'être affécté au noeud en fonction
 de l'angle précédemment affécté */
bool adjustSensitivity(int id, float angle)
{
    int marge = 3;
    float diff;
    
    switch(id)
    {
        case 0 :
            diff = angle - NUQUE->lastAngle;
            if(NUQUE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                NUQUE->lastAngle = angle;
                return true;
            }
            break;
        case 1 :
            diff = angle - EPAULE_DROITE->lastAngle;
            if(EPAULE_DROITE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                EPAULE_DROITE->lastAngle = angle;
                return true;
            }
            break;
        case 2 :
            diff = angle - COUDE_DROIT->lastAngle;
            if(COUDE_DROIT->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                COUDE_DROIT->lastAngle = angle;
                return true;
            }
            break;
        case 3 :
            diff = angle - EPAULE_GAUCHE->lastAngle;
            if(EPAULE_GAUCHE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                EPAULE_GAUCHE->lastAngle = angle;
                return true;
            }
            break;
        case 4 :
            diff = angle - COUDE_GAUCHE->lastAngle;
            if(COUDE_GAUCHE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                COUDE_GAUCHE->lastAngle = angle;
                return true;
            }
            break;
        case 5 :
            diff = angle - GENOU_DROIT->lastAngle;
            if(GENOU_DROIT->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                GENOU_DROIT->lastAngle = angle;
                return true;
            }
            break;
        case 6 :
            diff = angle - GENOU_GAUCHE->lastAngle;
            if(GENOU_GAUCHE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                GENOU_GAUCHE->lastAngle = angle;
                return true;
            }
            break;
        case 7 :
            diff = angle - HANCHE_DROITE->lastAngle;
            if(HANCHE_DROITE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                HANCHE_DROITE->lastAngle = angle;
                return true;
            }
            break;
        case 8 :
            diff = angle - HANCHE_GAUCHE->lastAngle;
            if(HANCHE_GAUCHE->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                HANCHE_GAUCHE->lastAngle = angle;
                return true;
            }
            break;
        case 9 :
            diff = angle - BASE_COLONNE_V->lastAngle;
            if(BASE_COLONNE_V->lastAngle == 0 || diff >= marge || diff <= -marge)
            {
                BASE_COLONNE_V->lastAngle = angle;
                return true;
            }
            break;
        default :
            return false;
            break;
    }
    return false;
}

/*!\brief Fonction qui effectue la rotation de chaque noeud en fonction de l'angle calculé lors de la calibration des membres de l'utilisateur */
void affectAngleToNode(FbxNode* pNode)
{
     if(NUQUE->getName() != NULL  && EPAULE_DROITE->isAvailable() == true)
         if(strcmp(pNode->GetName(),NUQUE->getName()) == 0)
         {
             pNode->SetRotationActive(true);
             if(adjustSensitivity(0, NUQUE->angle) == true)
                pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, NUQUE->angle));
             //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
             pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
             return;
         }
 
    if(EPAULE_DROITE->getName() != NULL && EPAULE_DROITE->isAvailable() == true)
        if(strcmp(pNode->GetName(),EPAULE_DROITE->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(1, EPAULE_DROITE->angle) == true)
                if(currentAvatar != 1 && currentAvatar != 2)
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, -EPAULE_DROITE->angle+180));}
                else
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, EPAULE_DROITE->angle+180));}
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(COUDE_DROIT->getName() != NULL && COUDE_DROIT->isAvailable() == true)
        if(strcmp(pNode->GetName(),COUDE_DROIT->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(2, COUDE_DROIT->angle) == true)
                if(currentAvatar != 1 && currentAvatar != 2)
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, -COUDE_DROIT->angle-180));}
                else
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, COUDE_DROIT->angle-180));}
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(EPAULE_GAUCHE->getName() != NULL && EPAULE_GAUCHE->isAvailable() == true)
        if(strcmp(pNode->GetName(),EPAULE_GAUCHE->getName()) == 0)
        {
            pNode->SetRotationActive(true);
             if(adjustSensitivity(3, EPAULE_GAUCHE->angle) == true)
                 pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, EPAULE_GAUCHE->angle+180));
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(COUDE_GAUCHE->getName() != NULL && COUDE_GAUCHE->isAvailable() == true)
        if(strcmp(pNode->GetName(),COUDE_GAUCHE->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(4, COUDE_GAUCHE->angle) == true)
                pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, COUDE_GAUCHE->angle+180));
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(GENOU_DROIT->getName() != NULL && GENOU_DROIT->isAvailable() == true)
        if(strcmp(pNode->GetName(),GENOU_DROIT->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(5, GENOU_DROIT->angle) == true)
                pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, GENOU_DROIT->angle-180));
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(GENOU_GAUCHE->getName() != NULL && GENOU_GAUCHE->isAvailable() == true)
        if(strcmp(pNode->GetName(),GENOU_GAUCHE->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(6, GENOU_GAUCHE->angle) == true)
                pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, -GENOU_GAUCHE->angle+180));
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(HANCHE_DROITE->getName() != NULL && HANCHE_DROITE->isAvailable() == true)
        if(strcmp(pNode->GetName(),HANCHE_DROITE->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            //pNode->SetPreRotation(FbxNode::eSourcePivot, FbxVector4(90, 90, 180));
            if(adjustSensitivity(7, HANCHE_DROITE->angle) == true)
                if(currentAvatar != 1 && currentAvatar != 2)
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, -HANCHE_DROITE->angle));}
                else
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, HANCHE_DROITE->angle));}
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }

    if(HANCHE_GAUCHE->getName() != NULL && HANCHE_GAUCHE->isAvailable() == true)
        if(strcmp(pNode->GetName(),HANCHE_GAUCHE->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(8, HANCHE_GAUCHE->angle) == true)
                pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, HANCHE_GAUCHE->angle));
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
    
    if(BASE_COLONNE_V->getName() != NULL && BASE_COLONNE_V->isAvailable() == true)
        if(strcmp(pNode->GetName(),BASE_COLONNE_V->getName()) == 0)
        {
            pNode->SetRotationActive(true);
            if(adjustSensitivity(9, BASE_COLONNE_V->angle) == true)
                if(currentAvatar != 1 && currentAvatar != 2)
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, -BASE_COLONNE_V->angle));}
                else
                    {pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0, 0, BASE_COLONNE_V->angle));}
            //pNode->SetQuaternionInterpolation(FbxNode::eDestinationPivot, pNode->GetQuaternionInterpolation(FbxNode::eSourcePivot));
            pNode->ConvertPivotAnimationRecursive(NULL, FbxNode::eDestinationPivot, 10);
            return;
        }
}

// Draw recursively each node of the scene. To avoid recomputing 
// uselessly the global positions, the global position of each 
// node is passed to it's children while browsing the node tree.
// If the node is part of the given pose for the current scene,
// it will be drawn at the position specified in the pose, Otherwise
// it will be drawn at the given time.
void DrawNodeRecursive(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, FbxAMatrix& pParentGlobalPosition, FbxPose* pPose)
{
    FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, pTime, pPose, &pParentGlobalPosition);
    FbxAMatrix lGlobalOffPosition, lGeometryOffset;
    
    if (pNode->GetNodeAttribute())
    {
        //fprintf(stderr,"%s\n",pNode->GetName());
        //Gemotry du noeud courant
        lGeometryOffset = GetGeometry(pNode);
        //Actualisation de la position du noeud en fonction de la position de dessin du parent + de la géométrie du parent
        lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
        
        affectAngleToNode(pNode);
        DrawNode(pNode, pTime, pAnimLayer, pParentGlobalPosition, lGlobalOffPosition, pPose);
    }

    //Parcours récursif des noeuds enfant si il y en a
    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
      DrawNodeRecursive(pNode->GetChild(lChildIndex), pTime, pAnimLayer, lGlobalPosition, pPose);
    }
}

// Draw the node following the content of it's node attribute.
void DrawNode(FbxNode* pNode, 
              FbxTime& pTime, 
              FbxAnimLayer* pAnimLayer,
              FbxAMatrix& pParentGlobalPosition,
              FbxAMatrix& pGlobalPosition, FbxPose* pPose)
{
  FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
    
    
  if (lNodeAttribute)
  {
        // All lights has been processed before the whole scene because they influence every geometry.
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
          DrawMesh(pNode, pTime, pAnimLayer, pGlobalPosition, pPose);
        }
  }
}

// Draw the vertices of a mesh.
void DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, FbxAMatrix& pGlobalPosition, FbxPose* pPose)
{

  FbxMesh* lMesh = pNode->GetMesh();
  const int lVertexCount = lMesh->GetControlPointsCount();
  
  INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
  // No vertex to draw.
  if (lVertexCount == 0) return;

  const VBOMesh * lMeshCache = static_cast<const VBOMesh *>(lMesh->GetUserDataPtr());
  
    
  // If it has some defomer connection, update the vertices position
  const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
    (static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
  const bool lHasShape = lMesh->GetShapeCount() > 0;
  const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
  const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;


  FbxVector4* lVertexArray = NULL;
  if (!lMeshCache || lHasDeformation)
  {
    lVertexArray = new FbxVector4[lVertexCount];
    memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
  }

  // Par defaut on dit qu'il n'y a pas de deformation. 
  // Cet etat sera modifie dans ComputeXXXDeformation
  GLint pId;
  glGetIntegerv(GL_CURRENT_PROGRAM, &pId);
  glUniform1i(glGetUniformLocation(pId, "ClusterMode"), -1);

    
  if (lHasDeformation)
  {
        // Active vertex cache deformer will overwrite any other deformer

        if (lHasVertexCache)
        {
          ReadVertexCacheData(lMesh, pTime, lVertexArray);
          INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
        }
        else
        {
              if (lHasShape)
              {
                // Deform the vertex array with the shapes.
                ComputeShapeDeformation(lMesh, pTime, pAnimLayer, lVertexArray);
                INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
              }

              //we need to get the number of clusters
              const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
              int lClusterCount = 0;
              for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
              {
                  lClusterCount += ((FbxSkin *)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
              }
              if (lClusterCount)
              {
                // Deform the vertex array with the skin deformer.
                ComputeSkinDeformation(pGlobalPosition, lMesh, pTime, lVertexArray, pPose);
                INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
              }
        }
      
        if (lMeshCache) lMeshCache->UpdateVertexPosition(lMesh, lVertexArray);
  }
  
  float temp[16];
  double *t= (double*)pGlobalPosition;
  for (int i=0; i<16; i++){
	temp[i] = (float)t[i];
  }
  glUniformMatrix4fv(glGetUniformLocation(pId, "FBXGlobalMatrix"), 1, GL_FALSE, temp);

  if (lMeshCache)
  {
    lMeshCache->BeginDraw();
    const int lSubMeshCount = lMeshCache->GetSubMeshCount();
    for (int lIndex = 0; lIndex < lSubMeshCount; ++lIndex)
    {
      const FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lIndex);
      if (lMaterial)
      {
        const MaterialCache * lMaterialCache = static_cast<const MaterialCache *>(lMaterial->GetUserDataPtr());
        if (lMaterialCache)
        {
          lMaterialCache->SetCurrentMaterial();
        }
      }
      else
      {
        // Draw green for faces without material
        MaterialCache::SetDefaultMaterial();
      }
          lMeshCache->Draw(lIndex);
    }
    lMeshCache->EndDraw();
  }  
  delete [] lVertexArray;
}


// Deform the vertex array with the shapes contained in the mesh.
void ComputeShapeDeformation(FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, FbxVector4* pVertexArray) {
    int lVertexCount = pMesh->GetControlPointsCount();
    
    FbxVector4* lSrcVertexArray = pVertexArray;
    FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
    memcpy(lDstVertexArray, pVertexArray, lVertexCount * sizeof(FbxVector4));
    
    int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
    for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex) {
      FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
      
      int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
      for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex) {
	FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
	if(lChannel) {
	  // Get the percentage of influence on this channel.
	  FbxAnimCurve* lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
	  if (!lFCurve) continue;
	  double lWeight = lFCurve->Evaluate(pTime);
	  
	  /*
	    If there is only one targetShape on this channel, the influence is easy to calculate:
	    influence = (targetShape - baseGeometry) * weight * 0.01
	    dstGeometry = baseGeometry + influence
	    
	    But if there are more than one targetShapes on this channel, this is an in-between 
	    blendshape, also called progressive morph. The calculation of influence is different.
	    
	    For example, given two in-between targets, the full weight percentage of first target
	    is 50, and the full weight percentage of the second target is 100.
	    When the weight percentage reach 50, the base geometry is already be fully morphed 
	    to the first target shape. When the weight go over 50, it begin to morph from the 
	    first target shape to the second target shape.
	    
	    To calculate influence when the weight percentage is 25:
	    1. 25 falls in the scope of 0 and 50, the morphing is from base geometry to the first target.
	    2. And since 25 is already half way between 0 and 50, so the real weight percentage change to 
	    the first target is 50.
	    influence = (firstTargetShape - baseGeometry) * (25-0)/(50-0) * 100
	    dstGeometry = baseGeometry + influence
	    
	    To calculate influence when the weight percentage is 75:
	    1. 75 falls in the scope of 50 and 100, the morphing is from the first target to the second.
	    2. And since 75 is already half way between 50 and 100, so the real weight percentage change 
	    to the second target is 50.
	    influence = (secondTargetShape - firstTargetShape) * (75-50)/(100-50) * 100
	    dstGeometry = firstTargetShape + influence
	  */
	  
	  // Find the two shape indices for influence calculation according to the weight.
	  // Consider index of base geometry as -1.

	  int lShapeCount = lChannel->GetTargetShapeCount();
	  double* lFullWeights = lChannel->GetTargetShapeFullWeights();
	  
	  // Find out which scope the lWeight falls in.
	  int lStartIndex = -1;
	  int lEndIndex = -1;
	  for(int lShapeIndex = 0; lShapeIndex<lShapeCount; ++lShapeIndex) {
	    if(lWeight > 0 && lWeight <= lFullWeights[0]) {
	      lEndIndex = 0;
	      break;
	    }
	    if(lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex+1]) {
	      lStartIndex = lShapeIndex;
	      lEndIndex = lShapeIndex + 1;
	      break;
	    }
	  }

	  FbxShape* lStartShape = NULL;
	  FbxShape* lEndShape = NULL;
	  if(lStartIndex > -1) {
	    lStartShape = lChannel->GetTargetShape(lStartIndex);
	  }
	  if(lEndIndex > -1) {
	    lEndShape = lChannel->GetTargetShape(lEndIndex);
	  }
	  
	  //The weight percentage falls between base geometry and the first target shape.
	  if(lStartIndex == -1 && lEndShape) {
	    double lEndWeight = lFullWeights[0];
	    // Calculate the real weight.
	    lWeight = (lWeight/lEndWeight) * 100;
	    // Initialize the lDstVertexArray with vertex of base geometry.
	    memcpy(lDstVertexArray, lSrcVertexArray, lVertexCount * sizeof(FbxVector4));
	    for (int j = 0; j < lVertexCount; j++) {
	      // Add the influence of the shape vertex to the mesh vertex.
	      FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
	      lDstVertexArray[j] += lInfluence;
	    }	
	  }
	  //The weight percentage falls between two target shapes.
	  else if(lStartShape && lEndShape) {
	    double lStartWeight = lFullWeights[lStartIndex];
	    double lEndWeight = lFullWeights[lEndIndex];
	    // Calculate the real weight.
	    lWeight = ((lWeight-lStartWeight)/(lEndWeight-lStartWeight)) * 100;
	    // Initialize the lDstVertexArray with vertex of the previous target shape geometry.
	    memcpy(lDstVertexArray, lStartShape->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
	    for (int j = 0; j < lVertexCount; j++) {
	      // Add the influence of the shape vertex to the previous shape vertex.
	      FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lStartShape->GetControlPoints()[j]) * lWeight * 0.01;
	      lDstVertexArray[j] += lInfluence;
	    }	
	  }
	}//If lChannel is valid
      }//For each blend shape channel
    }//For each blend shape deformer

    memcpy(pVertexArray, lDstVertexArray, lVertexCount * sizeof(FbxVector4));
    
    delete [] lDstVertexArray;
}

//Compute the transform matrix that the cluster will transform the vertex.
void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, 
			       FbxMesh* pMesh,
			       FbxCluster* pCluster, 
			       FbxAMatrix& pVertexTransformMatrix,
			       FbxTime pTime, 
			       FbxPose* pPose) {
  FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

  FbxAMatrix lReferenceGlobalInitPosition;
  FbxAMatrix lReferenceGlobalCurrentPosition;
  FbxAMatrix lAssociateGlobalInitPosition;
  FbxAMatrix lAssociateGlobalCurrentPosition;
  FbxAMatrix lClusterGlobalInitPosition;
  FbxAMatrix lClusterGlobalCurrentPosition;
  
  FbxAMatrix lReferenceGeometry;
  FbxAMatrix lAssociateGeometry;
  FbxAMatrix lClusterGeometry;
  
  FbxAMatrix lClusterRelativeInitPosition;
  FbxAMatrix lClusterRelativeCurrentPositionInverse;
  
  if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel()) {
    pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
    // Geometric transform of the model
    lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
    lAssociateGlobalInitPosition *= lAssociateGeometry;
    lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);
    
    pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
    // Multiply lReferenceGlobalInitPosition by Geometric Transformation
    lReferenceGeometry = GetGeometry(pMesh->GetNode());
    lReferenceGlobalInitPosition *= lReferenceGeometry;
    lReferenceGlobalCurrentPosition = pGlobalPosition;
    
    // Get the link initial global position and the link current global position.
    pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
    // Multiply lClusterGlobalInitPosition by Geometric Transformation
    lClusterGeometry = GetGeometry(pCluster->GetLink());
    lClusterGlobalInitPosition *= lClusterGeometry;
    lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);
    
    // Compute the shift of the link relative to the reference.
    //ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
    pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
      lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
  }
  else {
    pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
    lReferenceGlobalCurrentPosition = pGlobalPosition;
    // Multiply lReferenceGlobalInitPosition by Geometric Transformation
    lReferenceGeometry = GetGeometry(pMesh->GetNode());
    lReferenceGlobalInitPosition *= lReferenceGeometry;
    
    // Get the link initial global position and the link current global position.
    pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
    lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);
    
    // Compute the initial position of the link relative to the reference.
    lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
    
    // Compute the current position of the link relative to the reference.
    lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

    // Compute the shift of the link relative to the reference.
    pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
  }
}

void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, 
			      FbxMesh* pMesh, 
			      FbxTime& pTime, 
			      FbxVector4* pVertexArray,
			      FbxPose* pPose) {

//   Etape 1 : 
//   on cree une matrice pour chaque Sommet appelee ClusterDeformation initialise a 0
//   on cree un tableau de poids pour tous les sommets (1 poids par element) initialise a 0
//   si on est en mode additive alors chaque matrice est a identite


  // All the links must have the same link mode.
  FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();
  
  int lVertexCount = pMesh->GetControlPointsCount();
  FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
  memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));
  
  double* lClusterWeight = new double[lVertexCount];
  memset(lClusterWeight, 0, lVertexCount * sizeof(double));
	
  if (lClusterMode == FbxCluster::eAdditive) {
    for (int i = 0; i < lVertexCount; ++i) {
      lClusterDeformation[i].SetIdentity();
    }
  }

  //Etape 2
  // For all skins and all clusters, accumulate their deformation and weight
  // on each vertices and store them in lClusterDeformation and lClusterWeight.
  int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
  for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex) {
    FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
    
    int lClusterCount = lSkinDeformer->GetClusterCount();
    for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex) {
	FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
	if (!lCluster->GetLink())
	  continue;
	
	FbxAMatrix lVertexTransformMatrix;
	ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);
	int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
	for (int k = 0; k < lVertexIndexCount; ++k) {            
	  int lIndex = lCluster->GetControlPointIndices()[k];

	  // Sometimes, the mesh can have less points than at the time of the skinning
	  // because a smooth operator was active when skinning but has been deactivated during export.
	  if (lIndex >= lVertexCount)
	    continue;
	  
	  double lWeight = lCluster->GetControlPointWeights()[k];
	  
	  if (lWeight == 0.0) continue;
	  
	  // Compute the influence of the link on the vertex.
	  FbxAMatrix lInfluence = lVertexTransformMatrix;
	  MatrixScale(lInfluence, lWeight);
	  
	  if (lClusterMode == FbxCluster::eAdditive) {    
	    // Multiply with the product of the deformations on the vertex.
	    MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
	    lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];
	    
	    // Set the link to 1.0 just to know this vertex is influenced by a link.
	    lClusterWeight[lIndex] = 1.0;
	  }
	  else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
	    {
	      // Add to the sum of the deformations on the vertex.
	      MatrixAdd(lClusterDeformation[lIndex], lInfluence);
	      
	      // Add to the sum of weights to either normalize or complete the vertex.
	      lClusterWeight[lIndex] += lWeight;
	    }
	}//For each vertex			
    }//lClusterCount
  }  

//   //Etape 3
//   //Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
  for (int i = 0; i < lVertexCount; i++) {
    FbxVector4 lSrcVertex = pVertexArray[i];
    FbxVector4& lDstVertex = pVertexArray[i];
    double lWeight = lClusterWeight[i];
    
    // Deform the vertex if there was at least a link with an influence on the vertex,
    if (lWeight != 0.0) {
      lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
      if (lClusterMode == FbxCluster::eNormalize) {
	// In the normalized link mode, a vertex is always totally influenced by the links. 
	lDstVertex /= lWeight;
      }
      else if (lClusterMode == FbxCluster::eTotalOne) {
	// In the total 1 link mode, a vertex can be partially influenced by the links. 
	lSrcVertex *= (1.0 - lWeight);
	lDstVertex += lSrcVertex;
      }
    } 
  }
  
  delete [] lClusterDeformation;
  delete [] lClusterWeight;
}

// Deform the vertex array in Dual Quaternion Skinning way.
void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, 
				      FbxMesh* pMesh, 
				      FbxTime& pTime, 
				      FbxVector4* pVertexArray,
				      FbxPose* pPose) {
  // All the links must have the same link mode.
  FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();
  
  int lVertexCount = pMesh->GetControlPointsCount();
  int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
  
  FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
  memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));
  
  double* lClusterWeight = new double[lVertexCount];
  memset(lClusterWeight, 0, lVertexCount * sizeof(double));
  
  // For all skins and all clusters, accumulate their deformation and weight
  // on each vertices and store them in lClusterDeformation and lClusterWeight.
  for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex) {
    FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
    int lClusterCount = lSkinDeformer->GetClusterCount();
    for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex) {
      FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
      if (!lCluster->GetLink()) continue;
      
      FbxAMatrix lVertexTransformMatrix;
      ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

      FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
      FbxVector4 lT = lVertexTransformMatrix.GetT();
      FbxDualQuaternion lDualQuaternion(lQ, lT);
      
      int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
      for (int k = 0; k < lVertexIndexCount; ++k) { 
	int lIndex = lCluster->GetControlPointIndices()[k];
	
	// Sometimes, the mesh can have less points than at the time of the skinning
	// because a smooth operator was active when skinning but has been deactivated during export.
	if (lIndex >= lVertexCount) continue;
	
	double lWeight = lCluster->GetControlPointWeights()[k];
	
	if (lWeight == 0.0) continue;
	
	// Compute the influence of the link on the vertex.
	FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
	if (lClusterMode == FbxCluster::eAdditive) {    
	  // Simply influenced by the dual quaternion.
	  lDQClusterDeformation[lIndex] = lInfluence;
	  
	  // Set the link to 1.0 just to know this vertex is influenced by a link.
	  lClusterWeight[lIndex] = 1.0;
	}
	else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
	  {
	    if(lClusterIndex == 0) {
	      lDQClusterDeformation[lIndex] = lInfluence;
	    }
	    else {
	      // Add to the sum of the deformations on the vertex.
	      // Make sure the deformation is accumulated in the same rotation direction. 
	      // Use dot product to judge the sign.
	      double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
	      if( lSign >= 0.0 ) {
		lDQClusterDeformation[lIndex] += lInfluence;
	      }
	      else {
		lDQClusterDeformation[lIndex] -= lInfluence;
	      }
	    }
	    // Add to the sum of weights to either normalize or complete the vertex.
	    lClusterWeight[lIndex] += lWeight;
	  }
      }//For each vertex
    }//lClusterCount
  }
  
  //Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
  for (int i = 0; i < lVertexCount; i++) {
    FbxVector4 lSrcVertex = pVertexArray[i];
    FbxVector4& lDstVertex = pVertexArray[i];
    double lWeightSum = lClusterWeight[i];
    
    // Deform the vertex if there was at least a link with an influence on the vertex,
    if (lWeightSum != 0.0) {
      lDQClusterDeformation[i].Normalize();
      lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);
      
      if (lClusterMode == FbxCluster::eNormalize) {
	// In the normalized link mode, a vertex is always totally influenced by the links. 
	lDstVertex /= lWeightSum;
      }
      else if (lClusterMode == FbxCluster::eTotalOne) {
	// In the total 1 link mode, a vertex can be partially influenced by the links. 
	lSrcVertex *= (1.0 - lWeightSum);
	lDstVertex += lSrcVertex;
      }
    } 
  }
  
  delete [] lDQClusterDeformation;
  delete [] lClusterWeight;
}


void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, 
			   FbxMesh* pMesh, 
			   FbxTime& pTime, 
			    FbxVector4* pVertexArray,
			   FbxPose* pPose) {
  FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
  FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();
  
  if(lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid) {
    INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
    ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
  }
  else if(lSkinningType == FbxSkin::eDualQuaternion) {
    INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
    ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
  }
  else if(lSkinningType == FbxSkin::eBlend) {
    INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
    int lVertexCount = pMesh->GetControlPointsCount();
    
    FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
    memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    
    FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
    memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    
    ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
    ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);
    
    // To blend the skinning according to the blend weights
    // Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
    // DQSVertex: vertex that is deformed by dual quaternion skinning method;
    // LinearVertex: vertex that is deformed by classic linear skinning method;
    int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
    for(int lBWIndex = 0; lBWIndex<lBlendWeightsCount; ++lBWIndex) {
      double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
      pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
    }
  }
}


void ReadVertexCacheData(FbxMesh* pMesh, 
                         FbxTime& pTime, 
                         FbxVector4* pVertexArray) {
  FbxVertexCacheDeformer* lDeformer     = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache*               lCache        = lDeformer->GetCache();
    int                     lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
    unsigned int            lVertexCount  = (unsigned int)pMesh->GetControlPointsCount();
    bool                    lReadSucceed  = false;
    float*                  lReadBuf      = NULL;
    unsigned int			BufferSize	  = 0;

    if (lDeformer->Type.Get() != FbxVertexCacheDeformer::ePositions)
      // only process positions
      return; 
    
    unsigned int Length = 0;
    lCache->Read(NULL, Length, FBXSDK_TIME_ZERO, lChannelIndex);
    if (Length != lVertexCount*3)
      // the content of the cache is by vertex not by control points (we don't support it here)
      return;

    lReadSucceed = lCache->Read(&lReadBuf, BufferSize, pTime, lChannelIndex);
    if (lReadSucceed) {
      unsigned int lReadBufIndex = 0;
      
      while (lReadBufIndex < 3*lVertexCount) {
	// In statements like "pVertexArray[lReadBufIndex/3].SetAt(2, lReadBuf[lReadBufIndex++])", 
	// on Mac platform, "lReadBufIndex++" is evaluated before "lReadBufIndex/3". 
	// So separate them.
	pVertexArray[lReadBufIndex/3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
	pVertexArray[lReadBufIndex/3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
	pVertexArray[lReadBufIndex/3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
      }
    }
}



// Scale all the elements of a matrix.
void MatrixScale(FbxAMatrix& pMatrix, double pValue) {
  int i,j;
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      pMatrix[i][j] *= pValue;
    }
  }
}


// Add a value to all the elements in the diagonal of the matrix.
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue) {
  pMatrix[0][0] += pValue;
  pMatrix[1][1] += pValue;
  pMatrix[2][2] += pValue;
  pMatrix[3][3] += pValue;
}


// Sum two matrices element by element.
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix) {
  int i,j;
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      pDstMatrix[i][j] += pSrcMatrix[i][j];
    }
  }
}
