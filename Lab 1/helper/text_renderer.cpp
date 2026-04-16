#include "text_renderer.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"

bool BitmapTextRenderer::loadCharsetFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open charset file: " << path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    charset = buffer.str();

    while (!charset.empty() &&
        (charset.back() == '\n' || charset.back() == '\r' || charset.back() == ' '))
    {
        charset.pop_back();
    }

    return !charset.empty();
}

void BitmapTextRenderer::buildBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BitmapTextRenderer::init(
    const std::string& vertPath,
    const std::string& fragPath,
    const std::string& texturePath,
    const std::string& charsetPath,
    int screenWidth,
    int screenHeight,
    int glyphW,
    int glyphH,
    int cols,
    int rowsCount
)
{
    /*glyphWidth = glyphW;
    glyphHeight = glyphH;
    columns = cols;
    rows = rowsCount;

    advanceX = static_cast<float>(glyphWidth);
    lineHeight = static_cast<float>(glyphHeight);*/

    glyphWidth = glyphW;
    glyphHeight = glyphH;
    columns = cols;
    rows = rowsCount;

    advanceX = 18.0f;
    lineHeight = 24.0f;

    if (!loadCharsetFile(charsetPath))
    {
        return;
    }

    try
    {
        //prog.compileShader(vertPath);
        //prog.compileShader(fragPath);
        prog.compileShader(vertPath.c_str());
        prog.compileShader(fragPath.c_str());
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    fontTex = Texture::loadTexture(texturePath);

    glBindTexture(GL_TEXTURE_2D, fontTex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &atlasWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &atlasHeight);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    buildBuffers();
    resize(screenWidth, screenHeight);

    prog.use();
    prog.setUniform("FontTex", 0);

    initialised = true;
}

void BitmapTextRenderer::resize(int screenWidth, int screenHeight)
{
    projection = glm::ortho(0.0f, static_cast<float>(screenWidth),
        0.0f, static_cast<float>(screenHeight));
}

void BitmapTextRenderer::renderText(
    const std::string& text,
    float x,
    float y,
    float scale,
    const glm::vec3& colour
)
{
    if (!initialised)
        return;

    prog.use();
    prog.setUniform("Projection", projection);
    prog.setUniform("TextColour", colour);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex);

    glBindVertexArray(vao);

    float startX = x;

    for (char c : text)
    {
        if (c == '\n')
        {
            x = startX;
            y -= lineHeight * scale;
            continue;
        }

        if (c == ' ')
        {
            x += advanceX * scale;
            continue;
        }

        std::size_t index = charset.find(c);
        if (index == std::string::npos)
        {
            x += advanceX * scale;
            continue;
        }

        int col = static_cast<int>(index % columns);
        int row = static_cast<int>(index / columns);

        float u0 = (col * glyphWidth) / static_cast<float>(atlasWidth);
        float v0 = (row * glyphHeight) / static_cast<float>(atlasHeight);
        float u1 = ((col + 1) * glyphWidth) / static_cast<float>(atlasWidth);
        float v1 = ((row + 1) * glyphHeight) / static_cast<float>(atlasHeight);

        // Sheet is arranged top-to-bottom, so flip the row for OpenGL UVs
        float flippedV0 = 1.0f - v1;
        float flippedV1 = 1.0f - v0;

        float w = glyphWidth * scale;
        float h = glyphHeight * scale;

        float vertices[6][4] = {
            { x,     y + h, u0, flippedV1 },
            { x,     y,     u0, flippedV0 },
            { x + w, y,     u1, flippedV0 },

            { x,     y + h, u0, flippedV1 },
            { x + w, y,     u1, flippedV0 },
            { x + w, y + h, u1, flippedV1 }
        };

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += advanceX * scale;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BitmapTextRenderer::renderTextShadowed(
    const std::string& text,
    float x,
    float y,
    float scale,
    const glm::vec3& colour,
    const glm::vec3& shadowColour,
    float shadowOffsetX,
    float shadowOffsetY
)
{
    renderText(text, x + shadowOffsetX, y + shadowOffsetY, scale, shadowColour);
    renderText(text, x, y, scale, colour);
}