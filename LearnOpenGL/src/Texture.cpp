#include "Texture.h"

#include "stb_image/stb_image.h"

Texture::Texture(const std::string& path) :
	m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr),
	m_Width(0), m_Height(0), m_BPP(0)
{
	// 垂直翻转图像
	stbi_set_flip_vertically_on_load(1); // 纹理坐标系和OpenGL的坐标系不同，加载时需要翻转
	m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4); // 强制4通道RGBA
	GLCall(glGenTextures(1, &m_RendererID));
	// 2D纹理
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
	
	// 必须指定下列四个值
	// 纹理缩小过滤，线性重新取样
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	// 纹理放大过滤
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// 纹理环绕方式，S轴和T轴分别设置为GL_CLAMP_TO_EDGE。S是水平环绕，T是垂直环绕。
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	// GL_CLAMP会将纹理坐标限制在0到1之间，超出部分会被截断为边界颜色。
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	// 纹理数据上传到GPU，参数分别是：
	// 目标纹理类型、mipmap级别、内部格式（不同于格式本身，这是OpenGL如何存储数据的方式）、
	// 宽度、高度、边界（必须为0）、数据格式（提供给OpenGL的格式）、
	// 数据类型、像素数据指针（若没有准备好数据，只是想在显存分配空间，此处可为nullptr）
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 
		0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
	// 生成mipmap，自动生成不同分辨率的纹理版本，提升性能和视觉质量
	//GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	// 纹理数据已经上传到GPU，可以释放本地存储
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	// 若此缓冲区包含数据
	if (m_LocalBuffer)
	{
		stbi_image_free(m_LocalBuffer);
	}
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
	// 激活指定的纹理单元
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
