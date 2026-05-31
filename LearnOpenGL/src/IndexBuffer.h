#pragma once

class IndexBuffer
{
private:
	// 通用取名
	unsigned int m_rendererID;
	// 一共有多少个索引
	unsigned int m_count;
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return m_count; }
};

