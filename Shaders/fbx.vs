#version 410

#define eNone -1
#define eAdditive 0
#define eNormalize 1
#define eTotalOne 2

uniform mat4 modelViewMatrix;
uniform mat4 FBXGlobalMatrix;
uniform mat4 projectionMatrix;
uniform vec2 cam;

layout (location = 0) in vec4 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;

out vec2 vsoTexCoord;
out vec3 vsoNormal;
out vec4 vsoModPosition;


void main(void) {
  
  vec4 vsAnimPos = vec4(vsiPosition);

  vsoNormal = vec3((transpose(inverse(modelViewMatrix * FBXGlobalMatrix)) * vec4(vsiNormal.xyz, 0.0)).xyz);
  
  //vsoModPosition =  vec4(modelViewMatrix * FBXGlobalMatrix * vsAnimPos);
 vsoModPosition = modelViewMatrix * vec4(vsiPosition.xyz, 1.0);
    gl_Position = projectionMatrix * modelViewMatrix * vec4(vsiPosition.xyz, 1);
    
     //gl_Position = projectionMatrix * modelViewMatrix * vec4(vsiPosition.xyz, 1.0);
  vsoTexCoord = vsiTexCoord + cam;
}
