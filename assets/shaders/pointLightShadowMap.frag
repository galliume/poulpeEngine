#version 450

layout(location = 0) in vec4 FragPos;

layout(location = 1) in vec3 lightPos;
layout(location = 2) in float far_plane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}  