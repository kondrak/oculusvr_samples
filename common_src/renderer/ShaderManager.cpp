#include "renderer/ShaderManager.hpp"
#include <fstream>

// shader uniform names
static const char* uniformNames[] = { "ModelViewProjectionMatrix",
                                      "TextureMatrix",
                                      "vertexColor" };

ShaderManager* ShaderManager::GetInstance()
{
    static ShaderManager instance;
    return &instance;
}

ShaderManager::~ShaderManager()
{
    DestroyShaders();
}


void ShaderManager::DestroyShaders()
{
    for (int i = 0; i < NUM_SHADERS; i++)
    {
        if (glIsProgram(m_shaderProgram[i].id))
        {
            glDeleteProgram(m_shaderProgram[i].id);
        }

        if (glIsShader(m_shaderProgram[i].vertShader))
        {
            glDeleteShader(m_shaderProgram[i].vertShader);
        }

        if (glIsShader(m_shaderProgram[i].fragShader))
        {
            glDeleteShader(m_shaderProgram[i].fragShader);
        }
    }
}


// load all shaders
void ShaderManager::LoadShaders()
{
    LoadShader(BasicShader, "../common_res/Basic.vsh", "../common_res/Basic.fsh");
    LoadShader(OVRFrustumShader, "../common_res/OVRFrustum.vsh", "../common_res/OVRFrustum.fsh");
    LoadShader(FontShader, "../common_res/Font.vsh", "../common_res/Font.fsh");
}

// use shader program
const ShaderProgram& ShaderManager::UseShaderProgram(ShaderName type)
{
    if (m_activeShader != type)
    {
        m_activeShader = type;
        glUseProgram(m_shaderProgram[type].id);
    }

    return m_shaderProgram[type];
}


std::string ShaderManager::ReadShaderFromFile(const char *filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        LOG_MESSAGE_ASSERT(false, "Cannot open input file: " << filename);
        return NULL;
    }

    std::string shaderSrc;
    std::string readStr;

    while (!file.eof())
    {
        std::getline(file, readStr);
        shaderSrc.append(readStr);
        shaderSrc.append("\n");
    }

    file.close();

    return shaderSrc;
}


void ShaderManager::CompileShader(GLuint *newShader, GLenum shaderType, const char *shaderSrc)
{
    /* Create and compile the shader object */
    *newShader = glCreateShader(shaderType);

    glShaderSource(*newShader, 1, &shaderSrc, NULL);
    glCompileShader(*newShader);

    /* Test if compilation succeeded */
    GLint ShaderCompiled;
    glGetShaderiv(*newShader, GL_COMPILE_STATUS, &ShaderCompiled);

    if (!ShaderCompiled)
    {
        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(*newShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(*newShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
        LOG_MESSAGE_ASSERT(false, pszInfoLog);

        delete[] pszInfoLog;
        glDeleteShader(*newShader);
    }
}

// create the actual shader program
bool ShaderManager::LinkShader(GLuint* const pProgramObject,
                               const GLuint VertexShader,
                               const GLuint FragmentShader)
{
    *pProgramObject = glCreateProgram();

    glAttachShader(*pProgramObject, FragmentShader);
    glAttachShader(*pProgramObject, VertexShader);

    // Link the program object
    GLint Linked;
    glLinkProgram(*pProgramObject);
    glGetProgramiv(*pProgramObject, GL_LINK_STATUS, &Linked);

    if (!Linked)
    {
        int i32InfoLogLength, i32CharsWritten;
        glGetProgramiv(*pProgramObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        char* pszInfoLog = new char[i32InfoLogLength];
        glGetProgramInfoLog(*pProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
        LOG_MESSAGE_ASSERT(false, pszInfoLog);

        delete[] pszInfoLog;
        return false;
    }

    glUseProgram(*pProgramObject);

    return true;
}


void ShaderManager::LoadShader(ShaderName shaderName, const char* vshFilename, const char *fshFilename)
{
    std::string vShaderSrc = ReadShaderFromFile(vshFilename);
    std::string fShaderSrc = ReadShaderFromFile(fshFilename);

    CompileShader(&m_shaderProgram[shaderName].vertShader, GL_VERTEX_SHADER, vShaderSrc.c_str());
    CompileShader(&m_shaderProgram[shaderName].fragShader, GL_FRAGMENT_SHADER, fShaderSrc.c_str());

    LinkShader(&m_shaderProgram[shaderName].id, m_shaderProgram[shaderName].vertShader, m_shaderProgram[shaderName].fragShader);

    glUniform1i(glGetUniformLocation(m_shaderProgram[shaderName].id, "sTexture"), 0);  // Texture unit 0 is the primary texture.

    // Store the location of uniforms for later use
    for (int j = 0; j < NUM_UNIFORMS; ++j)
    {
        m_shaderProgram[shaderName].uniforms[j] = glGetUniformLocation(m_shaderProgram[shaderName].id, uniformNames[j]);
    }
}