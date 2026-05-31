#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "src/Renderer.h"
#include "src/VertexBuffer.h"
#include "src/VertexBufferLayout.h"
#include "src/IndexBuffer.h"
#include "src/VertexArray.h"
#include "src/Shader.h"
#include "src/Texture.h"

// 设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void test4(GLFWwindow* window)
{
    // 此处新建一个作用域，使在调用glfwTerminate前析构ib
	// glfwTerminate会破坏OpenGL上下文，此时如果ib还没有被析构，那么在ib的析构函数中调用glDeleteBuffers就会出错，因为OpenGL上下文已经被破坏了
    {
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };

        unsigned int indices[] = {
            0,1,2,
            2,3,0,
        };
        
		// 设置混合函数，参数分别是源因子和目标因子，这里使用了常见的alpha混合模式
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		// 启用混合
		GLCall(glEnable(GL_BLEND));
        
        VertexArray va;
        VertexBuffer vb(vertices, 4 * 5 * sizeof(float));

        VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(2);
		va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

		Shader shader("res/shaders/Basic.shader");
		shader.Bind();
		shader.SetUniform4f("u_color", 0.2f, 0.3f, 0.8f, 1.0f);

		Texture texture("res/textures/awesomeface.png");
		texture.Bind(0);
		shader.SetUniform1i("u_texture", 0);

        // 取消绑定
		va.Unbind();
        shader.Unbind();
		vb.Unbind();
        ib.Unbind();

		Renderer renderer;

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
            renderer.Clear();

            // 前面取消了绑定，从此处可以绑定新的
            shader.Bind();
            shader.SetUniform4f("u_color", r, 0.3f, 0.8f, 1.0f);

			renderer.Draw(va, ib, shader);

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
        //glDeleteBuffers(1, &VBO);
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

int main()
{
    glfwInit();

    // 使用glfwWindowHint函数来配置GLFW
    // 第一个参数代表选项的名称，可以从很多以GLFW_开头的枚举值中选择；第二个参数接受一个整型，用来设置这个选项的值。
    // 这里告诉GLFW要使用的OpenGL版本是4.6，并且要使用核心模式（core profile）。核心模式意味着我们将只能使用OpenGL的最新功能，而不能使用任何已经废弃的功能。
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    // 如果我们使用的是Mac OS X系统，还需要加上下面这行代码来启用前向兼容模式（forward compatibility）。前向兼容模式告诉OpenGL我们不需要支持任何已经废弃的功能，这在Mac OS X上是必须的。
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // 创建一个窗口对象，前两个参数是窗口的宽和高，第三个参数是窗口的标题，
    // 第四个参数可以指定我们希望将窗口设置为全屏模式还是使用独立的窗口模式，第五个参数可以指定我们希望与另一个已经存在的OpenGL上下文共享资源。
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "创建GLFW窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // 启用垂直同步（V-Sync），这会使得每次交换缓冲区时等待屏幕刷新完成
    // 从而限制帧率与显示器的刷新率相匹配，避免画面撕裂。
	glfwSwapInterval(1);
    // GLAD是用来管理OpenGL函数指针的库，我们需要在调用任何OpenGL函数之前初始化GLAD。
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // 输出版本
    std::cout << glGetString(GL_VERSION) << std::endl;

    // 设置OpenGL的视口（viewport），告诉OpenGL渲染窗口的大小。第一个参数是视口的左下角在窗口中的位置，第二个参数是视口的宽和高。
    //glViewport(0, 0, 800, 600);

    // 注册一个回调函数，当窗口大小发生改变时，GLFW会调用这个函数来调整视口的大小，以确保渲染的内容能够正确显示。
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //test1(window);
    //test3(window);
    test4(window);
    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------