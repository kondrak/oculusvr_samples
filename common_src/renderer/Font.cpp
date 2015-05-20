#include "renderer/Font.hpp"
#include "renderer/RenderContext.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/CameraDirector.hpp"
#include "renderer/Texture.hpp"
#include "renderer/TextureManager.hpp"

extern RenderContext  g_renderContext;
extern CameraDirector g_cameraDirector;

static const int CHAR_WIDTH     = 8;
static const int CHAR_HEIGHT    = 9;
static const float CHAR_SPACING = 1.5f;


Font::Font(const char *tex) : m_scale(1.f, 1.f), m_position(0.0f, 0.0f, 0.0f), m_color(1.f, 1.f, 1.f, 1.f)
{    
    glGenVertexArrays(1, &m_fontVertexArray);
    glBindVertexArray(m_fontVertexArray);

    const float verts[] = { 0.0f,  0.0f, -1.0f,
                            0.0f, -1.0f, -1.0f,
                            1.0f,  0.0f, -1.0f,
                            1.0f, -1.0f, -1.0f };

    const int indices[] = { 0, 1, 2, 3 };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::FontShader);
    m_vertexPosAttr = glGetAttribLocation(shader.id, "inVertex");

    glEnableVertexAttribArray(m_vertexPosAttr);
    glVertexAttribPointer(m_vertexPosAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    m_texture = TextureManager::GetInstance()->LoadTexture(tex);

    // disable l. filter for this texture (it's bound in Load())
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Font::~Font()
{
    if (glIsBuffer(m_vertexBuffer))
        glDeleteBuffers(1, &m_vertexBuffer);

    if (glIsBuffer(m_indexBuffer))
        glDeleteBuffers(1, &m_indexBuffer);

    glDisableVertexAttribArray(m_vertexPosAttr);

    if (glIsVertexArray(m_fontVertexArray))
        glDeleteVertexArrays(1, &m_fontVertexArray);
}

void Font::renderAt(const Math::Vector3f &pos, int w, int h, int uo, int vo, const Math::Vector4f &color)
{
    LOG_MESSAGE_ASSERT(m_texture != NULL, "Trying to render with no texture?");

    Math::Matrix4f texMatrix, mvMatrix;

    Math::Translate(mvMatrix, pos.m_x, pos.m_y);
    Math::Scale(mvMatrix, 2.f * w / g_renderContext.height, 2.f * h / g_renderContext.height);
    Math::Scale(mvMatrix, m_scale.m_x, m_scale.m_y);

    Math::Scale(texMatrix, 1.f / m_texture->Width(), -1.f / m_texture->Height());
    Math::Translate(texMatrix, (float)uo, (float)-vo);
    Math::Scale(texMatrix, (float)w, (float)h);

    Math::Matrix4f MVP = g_cameraDirector.GetActiveCamera()->ProjectionMatrix() * mvMatrix;

    // update matrices
    const ShaderProgram &shader = ShaderManager::GetInstance()->GetActiveShader();
    glUniformMatrix4fv(shader.uniforms[TextureMatrix], 1, GL_FALSE, &texMatrix[0]);
    glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVP[0]);
    glUniform4fv(shader.uniforms[VertexColor], 1, &color.m_x);

    glBindVertexArray(m_fontVertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
}

void Font::drawText(const std::string &text, float x, float y, float z, float r, float g, float b, float a)
{
    drawText(text, Math::Vector3f(x, y, z), Math::Vector4f(r, g, b, a));
}

void Font::drawText(const std::string &text, float x, float y, float z)
{
    drawText(text, Math::Vector3f(x, y, z), m_color);
}

void Font::drawText(const std::string &text)
{
    drawText(text, m_position, m_color);
}

void Font::drawText(const std::string &text, const Math::Vector3f &position, const Math::Vector4f &color)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Camera::CameraMode camMode = g_cameraDirector.GetActiveCamera()->GetMode();
    g_cameraDirector.GetActiveCamera()->SetMode(Camera::CAM_ORTHO);

    TextureManager::GetInstance()->BindTexture(m_texture);
    ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::FontShader);

    Math::Vector3f pos = position;

    for (size_t i = 0; i < text.length(); i++)
    {
        int cu = text[i] - 32;

        if (cu >= 0 && cu < 32 * 3)
        {
            renderAt(pos, CHAR_WIDTH, CHAR_HEIGHT, cu % 16 * CHAR_WIDTH, cu / 16 * (CHAR_HEIGHT + 1), color);
        }

        pos.m_x += m_scale.m_x * (CHAR_SPACING / g_renderContext.scrRatio) * CHAR_WIDTH / g_renderContext.height;
    }

    g_cameraDirector.GetActiveCamera()->SetMode(camMode);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

