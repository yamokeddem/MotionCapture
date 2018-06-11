/****************************************************************************************

Copyright (C) 2014 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#include "SceneCache.h"

#ifdef DEBUG
#define INFO if(1) printf
#else 
#define INFO if(0) printf
#endif

namespace
{
    const float ANGLE_TO_RADIAN = 3.1415926f / 180.f;
    const GLfloat BLACK_COLOR[] = {0.0f, 0.0f, 0.0f, 1.0f};
    const GLfloat GREEN_COLOR[] = {0.0f, 1.0f, 0.0f, 1.0f};
    const GLfloat WHITE_COLOR[] = {1.0f, 1.0f, 1.0f, 1.0f};

    const int TRIANGLE_VERTEX_COUNT = 3;

    // Four floats for every position.
    const int VERTEX_STRIDE = 4;
    // Three floats for every normal.
    const int NORMAL_STRIDE = 3;
    // Two floats for every UV.
    const int UV_STRIDE = 2;

    // Get specific property value and connected texture if any.
    // Value = Property value * Factor property value (if no factor property, multiply by 1).
    FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,
        const char * pPropertyName,
        const char * pFactorPropertyName,
        GLuint & pTextureName) {

      FbxDouble3 lResult(0, 0, 0);
      const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
      const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
      if (lProperty.IsValid() && lFactorProperty.IsValid()) {
	lResult = lProperty.Get<FbxDouble3>();
	double lFactor = lFactorProperty.Get<FbxDouble>();
	if (lFactor != 1) {
	  lResult[0] *= lFactor;
	  lResult[1] *= lFactor;
	  lResult[2] *= lFactor;
	}
      }
      
      if (lProperty.IsValid()) {
	const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
	if (lTextureCount) {
	  const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
	  if (lTexture && lTexture->GetUserDataPtr()) {
	    pTextureName = *(static_cast<GLuint *>(lTexture->GetUserDataPtr()));
	  }
	}
      }
      
      return lResult;
    }
}

VBOMesh::VBOMesh() : mHasNormal(false), mHasUV(false), mAllByControlPoint(true)
{
  // Reset every VBO to zero, which means no buffer.
  for (int lVBOIndex = 0; lVBOIndex < VBO_COUNT; ++lVBOIndex) {
    mVBONames[lVBOIndex] = 0;
  }
}

VBOMesh::~VBOMesh()
{
  // Delete VBO objects, zeros are ignored automatically.
  glDeleteBuffers(VBO_COUNT, mVBONames);
  
  for(int i=0; i < mSubMeshes.GetCount(); i++){
    delete mSubMeshes[i];
  }
	
  mSubMeshes.Clear();
}

bool VBOMesh::Initialize(const FbxMesh *pMesh)
{
  if (!pMesh->GetNode())
    return false;
  
  const int lPolygonCount = pMesh->GetPolygonCount();
  
  // Count the polygon count of each material
  FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
  FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
  if (pMesh->GetElementMaterial()) {
    lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
    lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
    if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon) {
      FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
      if (lMaterialIndice->GetCount() == lPolygonCount) {
	// Count the faces of each material
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
	  const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
	  if (mSubMeshes.GetCount() < lMaterialIndex + 1) {
	    mSubMeshes.Resize(lMaterialIndex + 1);
	  }
	  if (mSubMeshes[lMaterialIndex] == NULL) {
	    mSubMeshes[lMaterialIndex] = new SubMesh;
	  }
	  mSubMeshes[lMaterialIndex]->TriangleCount += 1;
	}

	// Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
	// if, in the loop above, we resized the mSubMeshes by more than one slot.
	for (int i = 0; i < mSubMeshes.GetCount(); i++)  {
	  if (mSubMeshes[i] == NULL)
	    mSubMeshes[i] = new SubMesh;
	}

	// Record the offset (how many vertex)
	const int lMaterialCount = mSubMeshes.GetCount();
	int lOffset = 0;
	for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex) {
	  mSubMeshes[lIndex]->IndexOffset = lOffset;
	  lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
	  // This will be used as counter in the following procedures, reset to zero
	  mSubMeshes[lIndex]->TriangleCount = 0;
	}
	FBX_ASSERT(lOffset == lPolygonCount * 3);
      }
    }
  }

  // All faces will use the same material.
  if (mSubMeshes.GetCount() == 0) {
    mSubMeshes.Resize(1);
    mSubMeshes[0] = new SubMesh();
  }

  // Congregate all the data of a mesh to be cached in VBOs.
  // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
  mHasNormal = pMesh->GetElementNormalCount() > 0;
  mHasUV = pMesh->GetElementUVCount() > 0;
  FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
  FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
  if (mHasNormal) {
    lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
    if (lNormalMappingMode == FbxGeometryElement::eNone) {
      mHasNormal = false;
    }
    if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint) {
      mAllByControlPoint = false;
    }
  }
  if (mHasUV) {
    lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
    if (lUVMappingMode == FbxGeometryElement::eNone) {
      mHasUV = false;
    }
    if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint) {
      mAllByControlPoint = false;
    }
  }

  // Allocate the array memory, by control point or by polygon vertex.
  int lPolygonVertexCount = pMesh->GetControlPointsCount();
  if (!mAllByControlPoint) {
    lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
  }

  float * lVertices = new float[lPolygonVertexCount * VERTEX_STRIDE];
  unsigned int * lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
  float * lNormals = NULL;

  if (mHasNormal) {
    lNormals = new float[lPolygonVertexCount * NORMAL_STRIDE];
  }
  float * lUVs = NULL;
  FbxStringList lUVNames;
  pMesh->GetUVSetNames(lUVNames);
  const char * lUVName = NULL;
  if (mHasUV && lUVNames.GetCount()) {
    lUVs = new float[lPolygonVertexCount * UV_STRIDE];
    lUVName = lUVNames[0];
  }

  // Populate the array with vertex attribute, if by control point.
  const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
  FbxVector4 lCurrentVertex;
  FbxVector4 lCurrentNormal;
  FbxVector2 lCurrentUV;
  if (mAllByControlPoint) {
    const FbxGeometryElementNormal * lNormalElement = NULL;
    const FbxGeometryElementUV * lUVElement = NULL;
    if (mHasNormal) {
      lNormalElement = pMesh->GetElementNormal(0);
    }
    if (mHasUV) {
      lUVElement = pMesh->GetElementUV(0);
    }
    for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex) {
      // Save the vertex position.
      lCurrentVertex = lControlPoints[lIndex];
      lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
      lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
      lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
      lVertices[lIndex * VERTEX_STRIDE + 3] = 1;

      // Save the normal.
      if (mHasNormal) {
	int lNormalIndex = lIndex;
	if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
	  lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
	}
	lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
	lNormals[lIndex * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
	lNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
	lNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
      }
      
      // Save the UV.
      if (mHasUV) {
	int lUVIndex = lIndex;
	if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
	  lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
	}
	lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
	lUVs[lIndex * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
	lUVs[lIndex * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
      }
    }
    
  }

  int lVertexCount = 0;
  for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
    // The material for current face.
    int lMaterialIndex = 0;
    if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon) {
      lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
    }
    
    // Where should I save the vertex attribute index, according to the material
    const int lIndexOffset = mSubMeshes[lMaterialIndex]->IndexOffset +
      mSubMeshes[lMaterialIndex]->TriangleCount * 3;
    for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex) {
      const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
      
      if (mAllByControlPoint) {
	lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
      }
      // Populate the array with vertex attribute, if by polygon vertex.
      else {
	lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);
	
	lCurrentVertex = lControlPoints[lControlPointIndex];
	lVertices[lVertexCount * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
	lVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
	lVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
	lVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;
	
	if (mHasNormal) {
	  pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
	  lNormals[lVertexCount * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
	  lNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
	  lNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
	}
	
	if (mHasUV) {
	  bool lUnmappedUV;
	  pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
	  lUVs[lVertexCount * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
	  lUVs[lVertexCount * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
	}
      }
      ++lVertexCount;
    }
    mSubMeshes[lMaterialIndex]->TriangleCount += 1;
  }

  //  Add by Vince(GL3.3) : Create VAO 
  glGenVertexArrays(1, &mVAOName);
  glBindVertexArray(mVAOName);

  // Create VBOs
  glGenBuffers(VBO_COUNT, mVBONames);

  // Save vertex attributes into GPU
  glBindBuffer(GL_ARRAY_BUFFER, mVBONames[VERTEX_VBO]);
  glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * VERTEX_STRIDE * sizeof(float), lVertices, GL_STATIC_DRAW);
   glVertexAttribPointer(0, VERTEX_STRIDE, GL_FLOAT, GL_FALSE, 0, ((const void*)(0)));
   glEnableVertexAttribArray(0);
  delete [] lVertices;


  if (mHasNormal) {
    glBindBuffer(GL_ARRAY_BUFFER, mVBONames[NORMAL_VBO]);
    glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * NORMAL_STRIDE * sizeof(float), lNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, NORMAL_STRIDE, GL_FLOAT, GL_FALSE, 0, ((const void*)(0)));
    glEnableVertexAttribArray(1);
    delete [] lNormals;
  }
    
    if (mHasUV) {
      glBindBuffer(GL_ARRAY_BUFFER, mVBONames[UV_VBO]);
      glBufferData(GL_ARRAY_BUFFER, lPolygonVertexCount * UV_STRIDE * sizeof(float), lUVs, GL_STATIC_DRAW);
     glVertexAttribPointer(2, UV_STRIDE, GL_FLOAT, GL_FALSE, 0, ((const void*)(0)));
     glEnableVertexAttribArray(2);

      delete [] lUVs;
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBONames[INDEX_VBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lPolygonCount * TRIANGLE_VERTEX_COUNT * sizeof(unsigned int), lIndices, GL_STATIC_DRAW);
    delete [] lIndices;    

    // Add by Vince(GL3.3) : Restore the context
    glBindVertexArray(0);

    return true;
}

void VBOMesh::UpdateVertexPosition(const FbxMesh * pMesh, const FbxVector4 * pVertices) const
{
    // Convert to the same sequence with data in GPU.
    float * lVertices = NULL;
    int lVertexCount = 0;

    INFO("%s %s %d\n", __FILE__, __func__, __LINE__);
    if (mAllByControlPoint) {
      lVertexCount = pMesh->GetControlPointsCount();
      lVertices = new float[lVertexCount * VERTEX_STRIDE];
      for (int lIndex = 0; lIndex < lVertexCount; ++lIndex) {
	lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(pVertices[lIndex][0]);
	  lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(pVertices[lIndex][1]);
	  lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(pVertices[lIndex][2]);
	  lVertices[lIndex * VERTEX_STRIDE + 3] = 1;
      }
    }
    else {
      const int lPolygonCount = pMesh->GetPolygonCount();
      lVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
      lVertices = new float[lVertexCount * VERTEX_STRIDE];
      
      int lVertexCount = 0;
      for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
	for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex) {
	  const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
	  lVertices[lVertexCount * VERTEX_STRIDE] = static_cast<float>(pVertices[lControlPointIndex][0]);
	  lVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(pVertices[lControlPointIndex][1]);
	  lVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(pVertices[lControlPointIndex][2]);
	  lVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;
	  ++lVertexCount;
	}
      }
    }
    // Transfer into GPU.
    if (lVertices) {
    BeginDraw();
    static GLuint tId = 0;
    if (tId ==0) glGenBuffers(1, &tId);
      glBindBuffer(GL_ARRAY_BUFFER, tId);
      glBufferData(GL_ARRAY_BUFFER, lVertexCount * VERTEX_STRIDE * sizeof(float), lVertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, VERTEX_STRIDE, GL_FLOAT, GL_FALSE, 0, ((const void*)(0)));
     EndDraw();
      delete [] lVertices;
    }
}

void VBOMesh::Draw(int pMaterialIndex) const {
    // Where to start.
  GLsizei lOffset = mSubMeshes[pMaterialIndex]->IndexOffset * sizeof(unsigned int);
  const GLsizei lElementCount = mSubMeshes[pMaterialIndex]->TriangleCount * 3;
  glDrawElements(GL_TRIANGLES, lElementCount, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(lOffset));
}

void VBOMesh::BeginDraw() const {
  // Add by Vince(GL3.3) : Bind the VAO
  glBindVertexArray(mVAOName);
}

void VBOMesh::EndDraw() const
{

  // Add by Vince(GL3.3) : Reset the VAO
  glBindVertexArray(0);
}

MaterialCache::MaterialCache() : mShinness(0)
{

}

MaterialCache::~MaterialCache()
{

}

// Bake material properties.
bool MaterialCache::Initialize(const FbxSurfaceMaterial * pMaterial)
{
    const FbxDouble3 lEmissive = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, mEmissive.mTextureName);
    mEmissive.mColor[0] = static_cast<GLfloat>(lEmissive[0]);
    mEmissive.mColor[1] = static_cast<GLfloat>(lEmissive[1]);
    mEmissive.mColor[2] = static_cast<GLfloat>(lEmissive[2]);

    const FbxDouble3 lAmbient = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, mAmbient.mTextureName);
    mAmbient.mColor[0] = static_cast<GLfloat>(lAmbient[0]);
    mAmbient.mColor[1] = static_cast<GLfloat>(lAmbient[1]);
    mAmbient.mColor[2] = static_cast<GLfloat>(lAmbient[2]);

    const FbxDouble3 lDiffuse = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, mDiffuse.mTextureName);
    mDiffuse.mColor[0] = static_cast<GLfloat>(lDiffuse[0]);
    mDiffuse.mColor[1] = static_cast<GLfloat>(lDiffuse[1]);
    mDiffuse.mColor[2] = static_cast<GLfloat>(lDiffuse[2]);

    const FbxDouble3 lSpecular = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, mSpecular.mTextureName);
    mSpecular.mColor[0] = static_cast<GLfloat>(lSpecular[0]);
    mSpecular.mColor[1] = static_cast<GLfloat>(lSpecular[1]);
    mSpecular.mColor[2] = static_cast<GLfloat>(lSpecular[2]);

    FbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        double lShininess = lShininessProperty.Get<FbxDouble>();
        mShinness = static_cast<GLfloat>(lShininess);
    }

    return true;
}


void MaterialCache::SetCurrentMaterial() const
{
  GLint pId;
  glGetIntegerv(GL_CURRENT_PROGRAM, &pId);
  glUniform4fv(glGetUniformLocation(pId, "Emissive"), 1,mEmissive.mColor);
  glUniform4fv(glGetUniformLocation(pId, "Ambient"), 1, mAmbient.mColor);
  glUniform4fv(glGetUniformLocation(pId, "Diffuse"), 1, mDiffuse.mColor);
  glUniform4fv(glGetUniformLocation(pId, "Specular"), 1,mSpecular.mColor);
  glUniform1f(glGetUniformLocation(pId, "Shininness"),mShinness);
   
  glActiveTexture(GL_TEXTURE0); 
  glBindTexture(GL_TEXTURE_2D, mDiffuse.mTextureName);

}

void MaterialCache::SetDefaultMaterial()
{
  GLint pId;
  glGetIntegerv(GL_CURRENT_PROGRAM, &pId);
  glUniform4fv(glGetUniformLocation(pId, "Emissive"),1, BLACK_COLOR);
  glUniform4fv(glGetUniformLocation(pId, "Ambient"), 1, BLACK_COLOR);
  glUniform4fv(glGetUniformLocation(pId, "Diffuse"), 1, GREEN_COLOR);
  glUniform4fv(glGetUniformLocation(pId, "Specular"),1, BLACK_COLOR); 
  glUniform1f(glGetUniformLocation(pId, "Shininness"),0);
    
  glActiveTexture(GL_TEXTURE0); 
  glBindTexture(GL_TEXTURE_2D, 0);
}

