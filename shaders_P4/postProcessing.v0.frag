#version 330 core

//Color salida 
layout (location = 0) out vec4 outColor;

//Texturas
uniform sampler2D colorTex;
uniform sampler2D vertexTex;
uniform sampler2D depthTex;

//Variables de control
uniform float focalDistance;
uniform float maxDistanceFactor;
uniform float near;
uniform float far;
uniform float time;
uniform bool useNoise;

//Variables Variantes 
in vec2 texCoord;

//Máscaras de convolución
uniform vec2 texIdx9[9];
uniform vec2 texIdx25[25];
uniform float mask9[9];
uniform float mask25[25];
uniform int maskSelector;

//Máscaras de convolución complementarias
const float horizontalFilterMask[9]= float[](
	-1.0,  -2.0,  -1.0,
	 0.0,   0.0,   0.0,
	 1.0,   2.0,   1.0);
const float verticalFilterMask[9]= float[](
	-1.0,   0.0,   1.0 ,
	-2.0,   0.0,   2.0 ,
	-1.0,   0.0,   1.0);
const float noiseIntensity = 25.0f;

vec4 noise(vec4);

void main()
{
	vec2 ts = vec2(1.0) / vec2(textureSize(colorTex, 0));

	//OBLIGATORIO 3
	//USO DEL BUFFER DE PROFUNDIDAD PARA EL DOF
	float vz = -(near * far) / (far + texture(depthTex, texCoord).x * (near - far));
	float dof = abs(vz - focalDistance) * maxDistanceFactor;
	//float dof = abs(texture(vertexTex, texCoord).z - focalDistance) * maxDistanceFactor;

	dof = clamp(dof, 0.0, 1.0);
	dof *= dof;

	vec4 color = vec4 (0.0);
	switch(maskSelector)
	{
		//Uso de la máscara por defecto 3x3
		case(0):
			for (int i = 0; i < 9; i++)
			{
				vec2 iidx = texCoord + ts * texIdx9[i] * dof;
				color += texture(colorTex, iidx, 0.0) * mask9[i];
			}
			break;

		//Uso de otras máscaras de convolución 3x3
		case(1):
		case(2):
		case(3):
			for (int i = 0; i < 9; i++)
			{
				vec2 iidx = texCoord + ts * texIdx9[i];
				color += texture(colorTex, iidx, 0.0) * mask9[i];
			}
		break;

		//OPCIONAL 1
		//CONCATENACIÓN DE DOS FILTROS GAUSIANOS
		case(4):
			for (int i = 0; i < 9; i++)
			{
				vec2 iidx = texCoord + ts * texIdx9[i];
				color += (texture(colorTex, iidx, 0.0) * verticalFilterMask[i]);
				color += (texture(colorTex, iidx, 0.0) * horizontalFilterMask[i]);
			}
			color *= 0.5;
		break;

		//Uso de una máscara de dimensiones 5x5
		case(5):
			for (int i = 0; i < 25; i++)
			{
				vec2 iidx = texCoord + ts * texIdx25[i] * dof;
				color += texture(colorTex, iidx, 0.0) * mask25[i];
			}
		break;
	}

	//OPCIONAL 2
	//USO DEL NOISE PARA CONCATENAR DISTINTOS POSTPROCESOS
	if(useNoise)
		color = noise(color);

	outColor = color;
}

vec4 noise(vec4 color)
{
	float x = (gl_FragCoord.x + 4) * (gl_FragCoord.y + 4) * (sin(time) * 10);
	vec4 noise = noiseIntensity * vec4(mod((mod(x, 13) + 1) * (mod(x, 123) + 1), 0.01) - 0.005);
	color += noise;
	return color;
}