#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "src/Renderer.h"
#include "src/VertexBuffer.h"
#include "src/IndexBuffer.h"
#include "src/VertexArray.h"
#include "src/Shader.h"


// 设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 清除错误
static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ")" << function <<
            " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

static ShaderProgramSource ParseShader(const std::string& file_path)
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

static unsigned int CompileShader(unsigned int type, const std::string& source)
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

static unsigned CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
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

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void test1(GLFWwindow* window)
{
    // 绘制窗口

    // 主循环（渲染循环），直到用户关闭窗口
    // glfwWindowShouldClose函数检查是否被要求关闭窗口，如果没有，就继续渲染
    while (!glfwWindowShouldClose(window))
    {
        // 传统GL代码
        //glClear(GL_COLOR_BUFFER_BIT);

        //glBegin(GL_TRIANGLES);
        //glVertex2f(-0.5f, -0.5f);
        //glVertex2f( 0.0f,  0.5f);
        //glVertex2f( 0.5f, -0.5f);
        //glEnd();

        // 检测外部输入
        processInput(window);

        // 渲染
        // 设置状态
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // 实际执行（状态使用）
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 终止GLFW，释放所有资源
    glfwTerminate();
}

void test2(GLFWwindow* window)
{
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    // 绘制三角形
    // "" ""之间会自动合并
    const std::string vertexShaderSource =
        "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";
    const char* fragmentShaderSource =
        "#version 460 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n";

    // build and compile our shader program
    // ------------------------------------
    // shader
    //unsigned int shaderProgram = CreateShader(vertexShaderSource, fragmentShaderSource);
    unsigned int shaderProgram = CreateShader(source.vertexShaderSource, source.fragmentShaderSource);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,

         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUseProgram(shaderProgram);

    GLCall(int location = glGetUniformLocation(shaderProgram, "u_color"));
    // 拼写错误，或者着色器中没有这个变量，都会导致location返回-1
    // 若是这个unform变量没有被使用，编译器会优化掉它，这时就无法获取到它的位置了，location也会返回-1
    ASSERT(location != -1);
    // 设置uniform变量,第一个参数是uniform变量在着色器程序中的位置，第二个参数是要设置的值
    GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

    // 指定该缓冲区的布局
    // 注意，此时OpenGL知道变量是一个vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // -----
        // 清楚屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        //glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        // 参数分别是：绘制模式、顶点数组起始索引、顶点数量
        // 状态机，前面已经绑定好了缓冲区和顶点属性，所以这里直接调用绘制函数就可以了
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // glBindVertexArray(0); // no need to unbind it every time 

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

void test3(GLFWwindow* window)
{
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    // build and compile our shader program
    // ------------------------------------
    // shader
    //unsigned int shaderProgram = CreateShader(vertexShaderSource, fragmentShaderSource);
    unsigned int shaderProgram = CreateShader(source.vertexShaderSource, source.fragmentShaderSource);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };

    unsigned int indices[] = {
        0,1,2,
        2,3,0,
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glUseProgram(shaderProgram);

    GLCall(int location = glGetUniformLocation(shaderProgram, "u_color"));
    // 拼写错误，或者着色器中没有这个变量，都会导致location返回-1
    // 若是这个unform变量没有被使用，编译器会优化掉它，这时就无法获取到它的位置了，location也会返回-1
    ASSERT(location != -1);
    // 设置uniform变量,第一个参数是uniform变量在着色器程序中的位置，第二个参数是要设置的值
    GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));
    // 指定该缓冲区的布局
    // 注意，此时OpenGL知道变量是一个vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // 取消绑定
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    float r = 0.0f;
    float increment = 1.0f / 120.0f;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // -----
        // 清除屏幕
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 前面取消了绑定，从此处可以绑定新的
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

        // draw our first triangle
        //glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        // 参数分别是：绘制模式、顶点数组起始索引、顶点数量
        // 状态机，前面已经绑定好了缓冲区和顶点属性，所以这里直接调用绘制函数就可以了
        // 与glDrawArrays不同，glDrawElements函数会根据我们在IBO中定义的索引来决定绘制哪些顶点，这样我们就可以重用顶点数据，避免重复定义相同的顶点。
        // 必须使用无符号整形
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f || r < 0.0f)
        {
            increment = -increment;
        }
        r += increment;
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}