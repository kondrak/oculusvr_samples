#include "leap/LeapMotion.hpp"
#include "leap/LeapListener.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/TextureManager.hpp"
#include "renderer/Texture.hpp"
#include "renderer/CameraDirector.hpp"
#include "renderer/RenderContext.hpp"

extern CameraDirector g_cameraDirector;
extern RenderContext g_renderContext;

// helper struct containing LM data and render data
struct LeapData
{
    LeapListener     m_listener;
    Leap::Controller m_controller;

    // skeletal hand data
    GLuint m_handVertexArray;
    GLuint m_handVertexBuffers[2];

    // camera image data
    GLuint   m_camImgVertexArray;
    GLuint   m_camImgVertexBuffer;
    GLuint   m_camImgColorBuffer;
    GLuint   m_camImgTexcoordBuffer;
    Texture *m_camImgTexture;
};

void LeapMotion::Init()
{
    m_leapData = new LeapData;
    m_leapData->m_camImgTexture = nullptr;
    m_leapData->m_controller.addListener(m_leapData->m_listener);

    SetupCameraImageTexture();
}

void LeapMotion::RecalculateSkeletonHands()
{
    Leap::HandList hands = m_leapData->m_controller.frame().hands();

    // reset hand vertex buffers - this will prevent hand rendering if no hands are detected
    if (glIsBuffer(m_leapData->m_handVertexBuffers[0]))
        glDeleteBuffers(1, &m_leapData->m_handVertexBuffers[0]);

    if (glIsBuffer(m_leapData->m_handVertexBuffers[1]))
        glDeleteBuffers(1, &m_leapData->m_handVertexBuffers[1]);

    if (!hands.count())
        return;

    // reset hand VAO
    if (glIsVertexArray(m_leapData->m_handVertexArray))
        glDeleteVertexArrays(1, &m_leapData->m_handVertexArray);
  
    glGenVertexArrays(1, &m_leapData->m_handVertexArray);
    glBindVertexArray(m_leapData->m_handVertexArray);

    for (int i = 0, numHands = hands.count(); i < numHands; i++)
    {
        Leap::Hand hand = hands[i];
        Leap::FingerList fingers = hand.fingers();

        const Leap::Vector palmPos    = hand.palmPosition();
        const Leap::Vector palmDir    = hand.direction();
        const Leap::Vector palmNormal = hand.palmNormal();
        const Leap::Vector palmSide   = palmDir.cross(palmNormal).normalized();

        const float thumbDist = hand.fingers()[Leap::Finger::TYPE_THUMB].bone(Leap::Bone::TYPE_METACARPAL).prevJoint().distanceTo(hand.palmPosition());
        const Leap::Vector wrist = palmPos - thumbDist * (palmDir * 0.9f + (hand.isLeft() ? -1.f : 1.f) * palmSide * 0.38f);

        Leap::Vector curBoxBase;
        Leap::Vector lastBoxBase = wrist;

        glGenBuffers(1, &m_leapData->m_handVertexBuffers[i]);

        // 5 fingers (indexed 0 - 4)
        // 4 bones per finger;
        // 6 floats per rendered segment (2 points);
        // 4 * 6 * 4 + 3 * 6 + 5 + 6
        GLfloat handVertexData[126];
        
        for (int j = 0, numFingers = fingers.count(); j < numFingers; j++)
        {
            const Leap::Finger &finger = fingers[j];

            for (int k = Leap::Bone::TYPE_METACARPAL; k <= Leap::Bone::TYPE_DISTAL; k++)
            {
                const Leap::Bone &bone = finger.bone(static_cast<Leap::Bone::Type>(k));

                // metacarpal box is rendered last (no visible bones in hand data)
                if (k == Leap::Bone::Type::TYPE_METACARPAL)
                {
                    curBoxBase = bone.nextJoint();
                }
                else
                {
                    // data offset: handVertexData[{fingerIdx} * {2 points} * {numBones} + {boneIdx} * {2 points} + {coordinate offset}]
                    handVertexData[j * 6 * 4 + k * 6 + 0] = bone.prevJoint().x;
                    handVertexData[j * 6 * 4 + k * 6 + 1] = bone.prevJoint().y;
                    handVertexData[j * 6 * 4 + k * 6 + 2] = bone.prevJoint().z;
                    handVertexData[j * 6 * 4 + k * 6 + 3] = bone.nextJoint().x;
                    handVertexData[j * 6 * 4 + k * 6 + 4] = bone.nextJoint().y;
                    handVertexData[j * 6 * 4 + k * 6 + 5] = bone.nextJoint().z;
                }
            }

            // add metacarpal segment data in the front of finger data batch (because metacarpal bone is indexed at 0)
            handVertexData[j * 6 * 4 + 0] = curBoxBase.x;
            handVertexData[j * 6 * 4 + 1] = curBoxBase.y;
            handVertexData[j * 6 * 4 + 2] = curBoxBase.z;
            handVertexData[j * 6 * 4 + 3] = lastBoxBase.x;
            handVertexData[j * 6 * 4 + 4] = lastBoxBase.y;
            handVertexData[j * 6 * 4 + 5] = lastBoxBase.z;

            lastBoxBase = curBoxBase;
        }

        // close the metacarpal box
        handVertexData[4 * 6 * 4 + 3 * 6 + 5 + 1] = wrist.x;
        handVertexData[4 * 6 * 4 + 3 * 6 + 5 + 2] = wrist.y;
        handVertexData[4 * 6 * 4 + 3 * 6 + 5 + 3] = wrist.z;
        handVertexData[4 * 6 * 4 + 3 * 6 + 5 + 4] = lastBoxBase.x;
        handVertexData[4 * 6 * 4 + 3 * 6 + 5 + 5] = lastBoxBase.y;
        handVertexData[4 * 6 * 4 + 3 * 6 + 5 + 6] = lastBoxBase.z;

        // bind hand vertices to buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_handVertexBuffers[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(handVertexData), handVertexData, GL_STATIC_DRAW);
    }
}

void LeapMotion::UpdateCameraImage()
{
    delete m_leapData->m_camImgTexture;
    m_leapData->m_camImgTexture = nullptr;

    // fetch latest Leap Motion camera images
    Leap::ImageList images = m_leapData->m_controller.frame().images();

    if (!images.count())
        return;

    // final image is monoscopic, so we only render one camera image
    Leap::Image image = images[0];

    // setup OpenGL texture from image data
    m_leapData->m_camImgTexture = new Texture((unsigned char*)image.data(), image.width(), image.height(), 3, GL_LUMINANCE, GL_LUMINANCE);
}

void LeapMotion::OnUpdate()
{
    RecalculateSkeletonHands();
    UpdateCameraImage();
    ProcessGestures();
}

void LeapMotion::OnRender()
{
    if (m_renderCameraImage)
        RenderCameraImage();
    else
        RenderSkeletonHands();
}

void LeapMotion::Destroy()
{
    if (!m_leapData)
        return;

    m_leapData->m_controller.removeListener(m_leapData->m_listener);

    if (glIsVertexArray(m_leapData->m_handVertexArray))
        glDeleteVertexArrays(1, &m_leapData->m_handVertexArray);

    if (glIsVertexArray(m_leapData->m_camImgVertexArray))
        glDeleteVertexArrays(1, &m_leapData->m_camImgVertexArray);

    if (glIsBuffer(m_leapData->m_handVertexBuffers[0]))
        glDeleteBuffers(1, &m_leapData->m_handVertexBuffers[0]);

    if (glIsBuffer(m_leapData->m_handVertexBuffers[1]))
        glDeleteBuffers(1, &m_leapData->m_handVertexBuffers[1]);

    if (glIsBuffer(m_leapData->m_camImgVertexBuffer))
        glDeleteBuffers(1, &m_leapData->m_camImgVertexBuffer);

    if (glIsBuffer(m_leapData->m_camImgColorBuffer))
        glDeleteBuffers(1, &m_leapData->m_camImgColorBuffer);

    if (glIsBuffer(m_leapData->m_camImgTexcoordBuffer))
        glDeleteBuffers(1, &m_leapData->m_camImgTexcoordBuffer);

    delete m_leapData->m_camImgTexture;
    delete m_leapData;

    m_leapData = nullptr;
}

void LeapMotion::ProcessGestures()
{
    const Leap::GestureList gestures = m_leapData->m_controller.frame().gestures();

    static bool swipeTracked = false;

    // on left/right swipe gesture switch between camera view and "ingame" hand rendering
    if (gestures.isEmpty() && swipeTracked)
    {
        m_renderCameraImage = !m_renderCameraImage;
        swipeTracked = false;
    }

    for (auto it = gestures.begin(); it != gestures.end(); ++it)
    {
        Leap::Gesture gesture = *it;

        switch (gesture.type())
        {
        case Leap::Gesture::TYPE_SWIPE:
            {
                Leap::Vector handDir = gesture.hands()[0].direction();

                // naive left/right swipe detection
                if (handDir.x > 0.5f || handDir.x < -0.5f)
                    swipeTracked = true;
            }
            break;
        default:
            break;
        }
    }
}

void LeapMotion::RenderSkeletonHands()
{
    // if the vertex array for hands is not set up, then we have no hands to render
    if (!glIsVertexArray(m_leapData->m_handVertexArray))
        return;

    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::OVRFrustumShader);
    GLuint vertexPositionAttr = glGetAttribLocation(shader.id, "inVertex");

    const float boneColor[4] = { 0.f, 1.f, 0.f, 1.0f };

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(m_leapData->m_handVertexArray);
    glEnableVertexAttribArray(vertexPositionAttr);

    // draw the hands
    for (int i = 0; i < 2; i++)
    {
        if (!glIsBuffer(m_leapData->m_handVertexBuffers[i]))
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_handVertexBuffers[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glUniform4fv(shader.uniforms[VertexColor], 1, boneColor);
        glDrawArrays(GL_LINES, 0, 126 / 3);
    }

    glDisableVertexAttribArray(vertexPositionAttr);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void LeapMotion::SetupCameraImageTexture()
{
    // origin is at screen center
    const GLfloat vertexBufferData[] = {
        -g_renderContext.scrRatio,  1.0f, -1.0f,
         g_renderContext.scrRatio,  1.0f, -1.0f,
        -g_renderContext.scrRatio, -1.0f, -1.0f,
         g_renderContext.scrRatio, -1.0f, -1.0f
    };

    const GLfloat vertexColorData[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    const GLfloat vertexTexcoordData[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    // create quad VAO and VBOs for camera texture quad
    glGenVertexArrays(1, &m_leapData->m_camImgVertexArray);
    glBindVertexArray(m_leapData->m_camImgVertexArray);

    glGenBuffers(1, &m_leapData->m_camImgVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_camImgVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_leapData->m_camImgColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_camImgColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColorData), vertexColorData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_leapData->m_camImgTexcoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_camImgTexcoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexcoordData), vertexTexcoordData, GL_STATIC_DRAW);
}

void LeapMotion::RenderCameraImage()
{
    if (!m_leapData->m_camImgTexture)
        return;

    m_leapData->m_camImgTexture->Load();

    g_cameraDirector.GetActiveCamera()->SetMode(Camera::CAM_ORTHO);
    Math::Matrix4f MVP = g_cameraDirector.GetActiveCamera()->ProjectionMatrix();

    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
    glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVP[0]);

    GLuint vertexPosition_modelspaceID = glGetAttribLocation(shader.id, "inVertex");
    GLuint vertexColorAttr = glGetAttribLocation(shader.id, "inVertexColor");
    GLuint texCoordAttr = glGetAttribLocation(shader.id, "inTexCoord");

    TextureManager::GetInstance()->BindTexture(m_leapData->m_camImgTexture);

    // OpenGL render 
    glBindVertexArray(m_leapData->m_camImgVertexArray);
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glEnableVertexAttribArray(vertexColorAttr);
    glEnableVertexAttribArray(texCoordAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_camImgVertexBuffer);
    glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_camImgColorBuffer);
    glVertexAttribPointer(vertexColorAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_leapData->m_camImgTexcoordBuffer);
    glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexColorAttr);
    glDisableVertexAttribArray(texCoordAttr);

    g_cameraDirector.GetActiveCamera()->SetMode(Camera::CAM_FPS);
}