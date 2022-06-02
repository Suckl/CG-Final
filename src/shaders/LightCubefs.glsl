#version 330 core

uniform float uLigIntensity;
uniform vec3 uLightColor;

void main(void) { gl_FragColor = vec4(uLightColor, 1.0); }