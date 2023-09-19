#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
	/*FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2f);*/
	FragColor = vec4(lightColor * objectColor, 1.0f);
}