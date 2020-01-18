#version 330 core
//Color salida 
layout (location = 0) out vec4 outColor;

//Textura 
uniform sampler2D colorTex;
uniform sampler2D vertexTex;
uniform sampler2D normalTex;
uniform sampler2D emiTex;


//Variables Variantes 
in vec2 texCoord;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
float alpha = 500.0;
vec3 Ke;
vec3 N;
vec3 pos;

//Propiedades de la luz
vec3 Ia = vec3 (0.3);
vec3 Id = vec3 (1.0);
vec3 Is = vec3 (0.7);
vec3 lpos = vec3 (0.0); 

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ke = texture(emiTex, texCoord).rgb;
	Ks = vec3 (1.0);

	N = texture(normalTex, texCoord).xyz;
	pos = texture(vertexTex, texCoord).xyz;

	outColor = vec4(shade(), 1.0);
}

vec3 shade()
{
	vec3 c = vec3(0.0);
	c = Ia * Ka;

	vec3 L = normalize (lpos - pos);
	vec3 diffuse = Id * Kd * dot (L,N);
	c += clamp(diffuse, 0.0, 1.0);
	
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max (dot (R,V), 0.01);
	vec3 specular = Is*Ks*pow(factor,alpha);
	c += clamp(specular, 0.0, 1.0);

	c += Ke;
	
	return c;
}