#version 330
out vec4 fragColor;
/* 4 floats venant du CPU, et affectés au vec4 couleur */
uniform vec4 couleur;

/* je récupère la sortie de mon vertex shader dans ce fragment shader */
in float il;

void main() {
  fragColor = vec4(il * couleur.rgb, couleur.a);
}
