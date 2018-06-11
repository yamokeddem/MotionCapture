#version 410

uniform sampler2D myTexture;

uniform vec4 lumpos;
uniform vec4 Emissive;
uniform vec4 Ambient;
uniform vec4 Diffuse;
uniform vec4 Specular;
uniform float Shininess;
uniform int lightFBX;
uniform vec4 couleur;
uniform int colorTexture;

in vec2 gsoTexCoord;
in vec3 gsoNormal;
in vec4 gsoModPosition;

out vec4 fragColor;

void main(void) {
  vec4 color;
  vec3 lum;
  if (lumpos.w == 1.0) {
    lum = normalize(gsoModPosition.xyz - lumpos.xyz);
  }
  else {
    lum  = -lumpos.xyz;
  }

  //Le 0.15 de ponderation pour l'ambient est fixé de maniere arbitraire
  color = dot(normalize(gsoNormal), normalize(-lum)) + 0.15 * Diffuse;
    
  vec4 tex = texture(myTexture, gsoTexCoord);

  if(lightFBX == 1)
      fragColor = mix(vec4(tex.rgb, tex.a), color, 1);
  else if(lightFBX == 0)
      fragColor = mix(vec4(tex.rgb, tex.a), color, 0);
 
  if(colorTexture == 1)
      fragColor = vec4(couleur.rgba);
}
