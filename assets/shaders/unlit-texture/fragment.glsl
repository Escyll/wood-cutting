#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);
    //FragColor = vec4(0.f, 0.f, 0.f, 1.f);
}
