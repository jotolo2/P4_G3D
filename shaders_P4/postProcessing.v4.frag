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

uniform vec2 texIdx9[9];
uniform vec2 texIdx25[25];
uniform float mask9[9];
uniform float mask25[25];
uniform int maskSize;
uniform int maskSelector;

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

	if(maskSelector < 4)
	{
		for (int i = 0; i < maskSize; i++)
		{
			vec2 iidx = texCoord + ts * texIdx9[i] * dof;
			color += texture(colorTex, iidx, 0.0) * mask9[i];
		}
	}else
	{
		for (int i = 0; i < maskSize; i++)
		{
			vec2 iidx = texCoord + ts * texIdx25[i] * dof;
			color += texture(colorTex, iidx, 0.0) * mask25[i];
		}
	}

	outColor = color;
}