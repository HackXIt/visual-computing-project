#version 330 core
in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragPos;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool useLighting;

void main()
{
    // Create a round sprite: discard fragments outside a circle
    float dist = length(gl_PointCoord - vec2(0.5));
    if(dist > 0.5)
        discard;

    if(useLighting)
    {
        // Compute ambient and diffuse lighting
        vec3 ambient = 0.2 * lightColor;
        vec3 norm = normalize(fragNormal);
        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        vec3 result = (ambient + diffuse) * fragColor;
        FragColor = vec4(result, 1.0);
    }
    else
    {
        // Lighting disabled: output the vertex color directly.
        FragColor = vec4(fragColor, 1.0);
    }
}
