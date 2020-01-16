#version 330 core
//Color salida 
layout (location = 0) out vec4 outColor;

//Textura 
uniform sampler2D colorTex;
uniform sampler2D vertexTex;

uniform float focalDistance; /* = -25.0*/
uniform float maxDistanceFactor; /*= 1.0/5.0*/
uniform float near;
uniform float far;

//Variables Variantes 
in vec2 texCoord;

#define MASK_LITTLE_SIZE 9u
const float maskLittleFactor = float (1.0/14.0);
const vec2 texIdxLittle[MASK_LITTLE_SIZE] = vec2[](
	vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0),
	vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0),
	vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0)); 
const float exampleMask[MASK_LITTLE_SIZE] = float[](
	float (1.0*maskLittleFactor), float (2.0*maskLittleFactor), float (1.0*maskLittleFactor),
	float (2.0*maskLittleFactor), float (2.0*maskLittleFactor), float (2.0*maskLittleFactor),
	float (1.0*maskLittleFactor), float (2.0*maskLittleFactor), float (1.0*maskLittleFactor));

	/*const float mask[MASK_LITTLE_SIZE] = float[](
	float (0.0), float (-1.0), float (0.0),
	float (0.0), float (1.0), float (0.0),
	float (0.0), float (0.0), float (0.0));*/
	

const float laplacianEdgeFilterMask[MASK_LITTLE_SIZE]= float[](
	 -1.0,-1.0,-1.0,
	 -1.0, 8.0,-1.0,
	 -1.0,-1.0,-1.0);
	 
const float northDirectionMask[MASK_LITTLE_SIZE]= float[](
	 -3.0,-3.0,5.0,
	 -3.0, 0.0,5.0,
	 -3.0,-3.0,5.0);
	 
const float embossFilterMask[MASK_LITTLE_SIZE]= float[](
	 2.0,   0.0 ,   0.0 ,
	 0.0,  -1.0 ,   0.0 ,
	 0.0,   0.0 ,  -1.0);
	 
const float horizontalFilterMask[MASK_LITTLE_SIZE]= float[](
	-1.0,   -2.0 ,   -1.0,
	 0.0,  0.0 ,   0.0 ,
	 1.0,   2.0 ,  1.0);

const float verticalFilterMask[MASK_LITTLE_SIZE]= float[](
	-1.0,   0.0 ,  1.0,
	 -2.0,  0.0 ,  2.0 ,
	-1.0,   0.0 ,  1.0);

const float identityMask[MASK_LITTLE_SIZE]= float[](
	 0.1,   0.1 ,   0.1 ,
	 0.1,  0.1 ,   0.1 ,
	 0.1,   0.1 ,  0.1);


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
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex, 0));

	float vz = -(near * far) / (far + texture(vertexTex, texCoord).z * (near - far)) ;
	//float dof = abs(texture(vertexTex, texCoord).r - focalDistance) * maxDistanceFactor;
	float dof = abs(vz - focalDistance) * maxDistanceFactor;
	dof = clamp (dof, 0.0, 1.0);
	dof *= dof;
	vec4 color = vec4 (0.0);
	vec4 colorOfConvolution=vec4 (0.0);

	for (uint i = 0u; i < MASK_LITTLE_SIZE; i++)
	{
		vec2 iidx = texCoord + ts * texIdxLittle[i];
		color += (texture(colorTex, iidx, 0.0) * verticalFilterMask[i]);
		color += (texture(colorTex, iidx, 0.0) * horizontalFilterMask[i]);
	}
	color= color*0.5;

	outColor = color;
}