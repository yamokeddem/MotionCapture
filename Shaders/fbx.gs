#version 410

in  vec3 vsoNormal[];
in  vec4 vsoModPosition[];
in  vec2 vsoTexCoord[];

out vec3 gsoNormal;
out vec2 gsoTexCoord;
out vec4 gsoModPosition;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main(void) {

  for(int i = 0; i < gl_in.length(); i++) {
    gl_Position = gl_in[i].gl_Position;
    gsoTexCoord = vsoTexCoord[i];
    gsoModPosition = vsoModPosition[i];
    gsoNormal = vsoNormal[i];
    EmitVertex();
  }
  EndPrimitive();
}
