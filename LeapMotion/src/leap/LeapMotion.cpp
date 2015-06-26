#include "leap/LeapMotion.hpp"
#include "leap/LeapListener.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/ShaderManager.hpp"

GLuint m_vertexArray;
//GLuint m_vertexBuffers[3];

GLuint m_fingerVertexBuffers[40];
GLuint m_metacarpalVertexBuffers[40];

extern Leap::HandList handList;

struct LeapData
{
    LeapListener     m_listener;
    Leap::Controller m_controller;
};

LeapMotion::~LeapMotion()
{
    delete m_leapData;
}

void LeapMotion::Init()
{
    m_leapData = new LeapData;
    m_leapData->m_controller.addListener(m_leapData->m_listener);
}

void LeapMotion::RecalculateSkeleton()
{
    // avoid asynchronous data invalidation by copying external list to local storage
    Leap::HandList hands = handList;
    if (!hands.count())
        return;

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);

    // create line VAO
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

    for (int i = 0; i < hands.count(); i++)
    {
        Leap::Hand hand = hands[i];
        Leap::FingerList fingers = hand.fingers();

        const Leap::Vector palmPos = hand.palmPosition();
        const Leap::Vector palmDir = hand.direction();
        const Leap::Vector palmNormal = hand.palmNormal();
        const Leap::Vector palmSide = palmDir.cross(palmNormal).normalized();

        const float thumbDist = hand.fingers()[Leap::Finger::TYPE_THUMB].bone(Leap::Bone::TYPE_METACARPAL).prevJoint().distanceTo(hand.palmPosition());
        const Leap::Vector wrist = palmPos - thumbDist * (palmDir * 0.9f + (hand.isLeft() ? -1.f : 1.f) * palmSide * 0.38f);

        Leap::Vector curBoxBase;
        Leap::Vector lastBoxBase = wrist;

        for (int j = 0; j < fingers.count(); j++)
        {
            const Leap::Finger &finger = fingers[j];

            for (int k = Leap::Bone::TYPE_METACARPAL; k <= Leap::Bone::TYPE_DISTAL; k++)
            {
                const Leap::Bone &bone = finger.bone(static_cast<Leap::Bone::Type>(k));

                if (k == Leap::Bone::Type::TYPE_METACARPAL)
                {
                    curBoxBase = bone.nextJoint();
                }
                else
                {
                    GLfloat fingerVertexData[] = { bone.prevJoint().x, bone.prevJoint().y, bone.prevJoint().z,
                        bone.nextJoint().x, bone.nextJoint().y, bone.nextJoint().z };

                    if (glIsBuffer(m_fingerVertexBuffers[k + j * 4 + i * 20]))
                        glDeleteBuffers(1, &m_fingerVertexBuffers[k + j * 4 + i * 20]);

                    glGenBuffers(1, &m_fingerVertexBuffers[k + j * 4 + i * 20]);

                    // Get a handle for our buffers (VBO)
                    glBindBuffer(GL_ARRAY_BUFFER, m_fingerVertexBuffers[k + j * 4 + i * 20]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(fingerVertexData), fingerVertexData, GL_STATIC_DRAW);
                }
            }

            // draw segment of metacarpal box
            if (glIsBuffer(m_metacarpalVertexBuffers[j + i * 6]))
                glDeleteBuffers(1, &m_metacarpalVertexBuffers[j + i * 6]);

            GLfloat mcVertexData[] = { curBoxBase.x, curBoxBase.y, curBoxBase.z,
                                       lastBoxBase.x, lastBoxBase.y, lastBoxBase.z };

            glGenBuffers(1, &m_metacarpalVertexBuffers[j + i * 6]);

            glBindBuffer(GL_ARRAY_BUFFER, m_metacarpalVertexBuffers[j + i * 6]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(mcVertexData), mcVertexData, GL_STATIC_DRAW);

            lastBoxBase = curBoxBase;
        }

        // close the metacarpal box
        if (glIsBuffer(m_metacarpalVertexBuffers[5 + i * 6]))
            glDeleteBuffers(1, &m_metacarpalVertexBuffers[5 + i * 6]);

        GLfloat mcVertexDataFinal[] = { wrist.x, wrist.y, wrist.z,
                                        lastBoxBase.x, lastBoxBase.y, lastBoxBase.z };

        glGenBuffers(1, &m_metacarpalVertexBuffers[5 + i * 6]);

        glBindBuffer(GL_ARRAY_BUFFER, m_metacarpalVertexBuffers[5 + i * 6]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(mcVertexDataFinal), mcVertexDataFinal, GL_STATIC_DRAW);
    }
}

void LeapMotion::OnRender()
{
    RecalculateSkeleton();

    if (!glIsVertexArray(m_vertexArray))
        return;

    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::OVRFrustumShader);
    GLuint vertexPositionAttr = glGetAttribLocation(shader.id, "inVertex");

    const float frustumColor[3] = { 0.f, 1.f, 0.f };

    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render camera bounds
    glBindVertexArray(m_vertexArray);
    glEnableVertexAttribArray(vertexPositionAttr);

    for (int i = 0; i < 40; i++)
    {
        if (!glIsBuffer(m_fingerVertexBuffers[i]))
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, m_fingerVertexBuffers[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // draw the frustum lines
        glUniform3fv(shader.uniforms[VertexColor], 1, frustumColor);
        glDrawArrays(GL_LINES, 0, 2);
    }

    for (int i = 0; i < 40; i++)
    {
        if (!glIsBuffer(m_metacarpalVertexBuffers[i]))
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, m_metacarpalVertexBuffers[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // draw the frustum lines
        glUniform3fv(shader.uniforms[VertexColor], 1, frustumColor);
        glDrawArrays(GL_LINES, 0, 2);
    }

    glDisableVertexAttribArray(vertexPositionAttr);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
}

void LeapMotion::Destroy()
{
    m_leapData->m_controller.removeListener(m_leapData->m_listener);
}