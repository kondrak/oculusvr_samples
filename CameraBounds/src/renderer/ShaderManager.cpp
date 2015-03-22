#include "renderer/ShaderManager.hpp"
#include <fstream>

// shader attribute names
static const char* aszAttribs[] = { "inVertex", "inVertexColor", "inTexCoord" };

// shader uniform names
static const char* aszUniformNames[] = { "ModelViewProjectionMatrix",
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
    LoadShader(BasicShader, "res/VertShaderBasic.vsh", "res/FragShaderBasic.fsh");
    LoadShader(FrustumShader, "res/VertShaderFrustum.vsh", "res/FragShaderFrustum.fsh");
}

// use shader program
const ShaderProgram& ShaderManager::UseShaderProgram(ShaderName type)
{
    if (m_activeShader != type)
    {
        m_activeShader = type;
        //CLAW_ASSERT(m_activeShader != NUM_SHADERS);

        glUseProgram(m_shaderProgram[type].id);
    }

    return m_shaderProgram[type];
}


char *ShaderManager::ReadShaderFromFile(const char *filename)
{
    std::ifstream file;
    std::streampos begin, end;

    file.open(filename);

    if (!file.is_open())
    {
        LOG_MESSAGE("Cannot open input file: " << filename);
        return NULL;
    }

    // get file size
    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();
    file.seekg(0, std::ios::beg);

    // allocate extra character for null terminator 
    char *shaderSrc = new char[(unsigned int)(end - begin) + 1];

    file.read(shaderSrc, end - begin);
    file.close();

    // properly terminate read string
    shaderSrc[end - begin] = '\0';

    return shaderSrc;
}


void ShaderManager::CompileShader(GLuint *newShader, GLenum shaderType, const char *shaderSrc)
{
    /* Create and compile the shader object */
    *newShader = glCreateShader(shaderType);

    //const char *src = vShaderSrc;
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
    const GLuint FragmentShader) /*,
                                 const char** const pszAttribs,
                                 const int i32NumAttribs)*/
{
    *pProgramObject = glCreateProgram();

    glAttachShader(*pProgramObject, FragmentShader);
    glAttachShader(*pProgramObject, VertexShader);

    for (int i = 0; i < 3; ++i)
    {
    glBindAttribLocation(*pProgramObject, i, aszAttribs[i]);
    }

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
    char *vShaderSrc = ReadShaderFromFile(vshFilename);
    char *fShaderSrc = ReadShaderFromFile(fshFilename);

    CompileShader(&m_shaderProgram[shaderName].vertShader, GL_VERTEX_SHADER, vShaderSrc);
    CompileShader(&m_shaderProgram[shaderName].fragShader, GL_FRAGMENT_SHADER, fShaderSrc);

    LinkShader(&m_shaderProgram[shaderName].id, m_shaderProgram[shaderName].vertShader, m_shaderProgram[shaderName].fragShader);

    // Store the location of uniforms for later use
    for (int j = 0; j < NUM_UNIFORMS; ++j)
    {
        m_shaderProgram[shaderName].uniforms[j] = glGetUniformLocation(m_shaderProgram[shaderName].id, aszUniformNames[j]);
    }

    delete[] vShaderSrc;
    delete[] fShaderSrc;
}