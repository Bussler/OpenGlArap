#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 vColor;
void main()
{
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	vColor = aColor;
};



#shader fragment
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main()
{
	FragColor = vec4(vColor, 1.0f);
};