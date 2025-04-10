#version 330 core
out vec4 FragColor;
uniform vec3 markerColor;
void main(){
    FragColor = vec4(markerColor, 1.0);
}
