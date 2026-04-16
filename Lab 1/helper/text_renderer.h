#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "glslprogram.h"

class BitmapTextRenderer
{
private:
    GLSLProgram prog;

    GLuint fontTex = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    glm::mat4 projection = glm::mat4(1.0f);

    std::string charset;

    int atlasWidth = 0;
    int atlasHeight = 0;

    int glyphWidth = 12;
    int glyphHeight = 12;

    int columns = 10;
    int rows = 11;

    float advanceX = 12.0f;
    float lineHeight = 12.0f;

    bool initialised = false;

    bool loadCharsetFile(const std::string& path);
    void buildBuffers();

public:
    BitmapTextRenderer() = default;

    void init(
        const std::string& vertPath,
        const std::string& fragPath,
        const std::string& texturePath,
        const std::string& charsetPath,
        int screenWidth,
        int screenHeight,
        int glyphW = 12,
        int glyphH = 12,
        int cols = 10,
        int rowsCount = 11
    );

    void resize(int screenWidth, int screenHeight);

    void renderText(
        const std::string& text,
        float x,
        float y,
        float scale = 1.0f,
        const glm::vec3& colour = glm::vec3(1.0f)
    );

    void renderTextShadowed(
        const std::string& text,
        float x,
        float y,
        float scale = 1.0f,
        const glm::vec3& colour = glm::vec3(1.0f),
        const glm::vec3& shadowColour = glm::vec3(0.0f),
        float shadowOffsetX = 2.0f,
        float shadowOffsetY = -2.0f
    );

    bool isInitialised() const { return initialised; }
};

#endif