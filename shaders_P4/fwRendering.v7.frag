#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosVertex;
layout(location = 2) out vec4 outNormalVertex;
layout(location = 3) out vec4 outEmiTex;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

uniform sampler2D colorTex;
uniform sampler2D emiTex;


void main()
{
	
	outPosVertex = vec4(pos.xyz, 0.0);

	outNormalVertex = vec4(normalize(norm), 1.0);

	outEmiTex = texture(emiTex, texCoord);

	outColor =  texture(colorTex, texCoord);
}
