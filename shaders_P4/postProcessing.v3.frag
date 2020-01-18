#version 330 core
//OBLIGATORIO 3
//Controlar el depth of field con el buffer de profundidad.

//Color salida 
layout (location = 0) out vec4 outColor;

//Textura 
uniform sampler2D colorTex;
uniform sampler2D vertexTex;
uniform sampler2D depthTex;

uniform float focalDistance; /* = -25.0*/
uniform float maxDistanceFactor; /*= 1.0/5.0*/
uniform float near;
uniform float far;

//Variables Variantes 
in vec2 texCoord;

#define MASK_SIZE 25u
const vec2 texIdx[MASK_SIZE] = vec2[](
	vec2(-2.0,2.0), vec2(-1.0,2.0), vec2(0.0,2.0), vec2(1.0,2.0), vec2(2.0,2.0),
	vec2(-2.0,1.0), vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0), vec2(2.0,1.0),
	vec2(-2.0,0.0), vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0), vec2(2.0,0.0),
	vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0), vec2(2.0,-1.0),
	vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2(0.0,-2.0), vec2(1.0,-2.0), vec2(2.0,-2.0));

const float maskFactor = float (1.0/65.0);

const float mask[MASK_SIZE] = float[](
	1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor, 2.0*maskFactor, 1.0*maskFactor,
	2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor, 3.0*maskFactor, 2.0*maskFactor,
	3.0*maskFactor, 4.0*maskFactor, 5.0*maskFactor, 4.0*maskFactor, 3.0*maskFactor,
	2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor, 3.0*maskFactor, 2.0*maskFactor,
	1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor, 2.0*maskFactor, 1.0*maskFactor);

void main()
{
	//Ser�a m�s r�pido utilizar una variable uniform el tama�o de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex, 0));

	float vz = -(near * far) / (far + texture(depthTex, texCoord).x * (near - far)) ;
	float dof = abs(vz - focalDistance) * maxDistanceFactor;

	dof = clamp (dof, 0.0, 1.0);
	dof *= dof;
	vec4 color = vec4 (0.0);

	for (uint i = 0u; i < MASK_SIZE; i++)
	{
		vec2 iidx = texCoord + ts * texIdx[i] * dof;
		color += texture(colorTex, iidx, 0.0) * mask[i];
	}

    outColor = color;

}