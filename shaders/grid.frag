#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;
layout(location = 3) in mat4 matView;
layout(location = 7) in mat4 matProj;
layout(location = 11) in float near;
layout(location = 12) in float far;

layout(location = 0) out vec4 outputColor;

vec4 grid(vec3 pos, float scale) {
    
	vec2 coord = pos.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    
	// z axis
    if(pos.x > -0.1 * minimumx && pos.x < 0.1 * minimumx)
        color.z = 1.0;
    
	// x axis
    if(pos.z > -0.1 * minimumz && pos.z < 0.1 * minimumz)
        color.x = 1.0;
    
	return color;
}

float depth(vec3 pos) {
    vec4 clipPos = matProj * matView * vec4(pos.xyz, 1.0);
    return (clipPos.z / clipPos.w);
}

float linearDepth(vec3 pos) {
    vec4 clipPos = matProj * matView * vec4(pos.xyz, 1.0);
    float clipDepth = (clipPos.z / clipPos.w) * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - clipDepth * (far - near));
    return linearDepth / far;
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
	vec3 pos = nearPoint + t * (farPoint - nearPoint);
	gl_FragDepth = depth(pos);
	float linearD = linearDepth(pos);
    float fading = max(0, (0.5 - linearD));
	
    outputColor = (grid(pos, 10) * float(t > 0) + grid(pos, 1)) * float(t > 0);
	outputColor.a *= fading;
}
