#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// #include "../Renderer/Renderer.h"


class Texture
{
    private:

        unsigned int m_RendererID;
        std::string m_filePath;
        unsigned char *m_LocalBuffer;
        int m_Width, m_Height, m_BPP;

    public:

        Texture(const std::string &filePath);
        ~Texture();

        void Bind(unsigned int slot = 0) const;
        void Unbind() const;

        inline int GetWidth() { return m_Width; }
        inline int GeHeight() { return m_Height; }
};