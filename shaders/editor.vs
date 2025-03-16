/*!\file editor.vs
 * \brief Vertex shader simplifié pour l'éditeur de circuit
 */

#version 330

layout(location = 0) in vec3 vsiPosition;
layout(location = 1) in vec3 vsiNormal;
layout(location = 2) in vec2 vsiTexCoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec2 vsoTexCoord;

void main(void) {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vsiPosition, 1.0);
    vsoTexCoord = vsiTexCoord;
}