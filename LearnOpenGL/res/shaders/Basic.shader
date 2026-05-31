#shader vertex
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 v_texCoord;

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    v_texCoord = texCoord;
};

#shader fragment
#version 460 core

in vec2 v_texCoord;

layout (location = 0) out vec4 color;

uniform vec4 u_color;
uniform sampler2D u_texture;

void main()
{
    vec4 tex_color = texture(u_texture, v_texCoord);
    color = u_color * tex_color;
};