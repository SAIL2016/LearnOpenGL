#include "Shader.h"
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>



Shader::Shader(const std::string& filename) :
	m_filePath(filename), m_rendererID(0)
{
    ShaderProgramSource source = ParseShader(filename);
	m_rendererID = CreateShader(source.vertexShaderSource, source.fragmentShaderSource);
}

Shader::~Shader()
{
	GLCall(glDeleteProgram(m_rendererID));
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_rendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    int location = GetUniformLocation(name);
    GLCall(glUniform1i(location, value));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    int location = GetUniformLocation(name);
    GLCall(glUniform4f(location, v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    int location = GetUniformLocation(name);
	// 参数分别是：uniform位置、矩阵数量、是否转置、矩阵数据指针
    GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
}

int Shader::GetUniformLocation(const std::string& name)
{
	if (m_uniformLocationsCache.find(name) != m_uniformLocationsCache.end())
	{
		return m_uniformLocationsCache[name];
	}

    GLCall(int location = glGetUniformLocation(m_rendererID, name.c_str()));
    // 返回-1不一定是出错，有可能没有用到
    if (location == -1)
    {
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
	// 不需要用else
    m_uniformLocationsCache[name] = location;
    return location;
}


ShaderProgramSource Shader::ParseShader(const std::string& file_path)
{
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };


    std::ifstream stream(file_path);
    std::stringstream ss[2];
    std::string line;
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                // vertex shader
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                // fragment shader
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    // 指定着色器的源码
    // 如果不想编译全部的字符串，可以使用第四个参数来指定每个字符串的长度
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // 错误处理
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        //不能这样创建，length是一个变量：char infoLog[length];
        //方法一，在堆上创建：char* infoLog = new char[length];
        //方法二，仍然在栈上创建
        char* message = (char*)alloca(length * sizeof(char));
        // 疑惑：第三个参数是什么？
        glGetShaderInfoLog(id, 512, NULL, message);
        std::cout << "ERROR::" <<
            (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") <<
            "SHADER::COMPILATION_FAILED" << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // 与OpenGL其他api接口并不十分一致
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    // 将着色器附加到程序对象上,就像C++一样，我们有两个文件，现在需要进行链接到程序中以便同时使用它们
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    // check for linking errors
    //glGetProgramiv(program, GL_LINK_STATUS, &success);
    //if (!success) {
    //    glGetProgramInfoLog(program, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    //}
    // 删除一些中间文件，就像删除C++的obj一样
    glDeleteShader(vs);
    glDeleteShader(fs);
    // 疑惑：不需要删除源码，原因？

    return program;
}