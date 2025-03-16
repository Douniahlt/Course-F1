/*!\file editor.fs
 * \brief Fragment shader simplifié pour l'éditeur de circuit
 */

#version 330

in vec2 vsoTexCoord;

uniform sampler2D textureSampler;
uniform float gridSize;

out vec4 fragColor;

void main(void) {
    vec2 scaledUV = vsoTexCoord * gridSize;
    fragColor = texture(textureSampler, scaledUV);
}