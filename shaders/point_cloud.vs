#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

out vec3 fragColor;

void main()
{
    fragColor = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = pointSize;
}
