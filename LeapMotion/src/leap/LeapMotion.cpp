#include "leap/LeapMotion.hpp"
#include "leap/LeapListener.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/ShaderManager.hpp"

GLuint m_vertexArray;
GLuint m_handVertexBuffer[2];

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
    Leap::HandList hands = handList;

    if (!hands.count())
        return;

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);

    // create line VAO
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

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

        // reset current hand vertex buffer
        if (glIsBuffer(m_handVertexBuffer[i]))
            glDeleteBuffers(1, &m_handVertexBuffer[i]);

        glGenBuffers(1, &m_handVertexBuffer[i]);

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
        glBindBuffer(GL_ARRAY_BUFFER, m_handVertexBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(handVertexData), handVertexData, GL_STATIC_DRAW);
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

    for (int i = 0; i < 2; i++)
    {
        if (!glIsBuffer(m_handVertexBuffer[i]))
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, m_handVertexBuffer[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // draw the frustum lines
        glUniform3fv(shader.uniforms[VertexColor], 1, frustumColor);
        glDrawArrays(GL_LINES, 0, 126 / 3);
    }

    glDisableVertexAttribArray(vertexPositionAttr);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
}

void LeapMotion::Destroy()
{
    m_leapData->m_controller.removeListener(m_leapData->m_listener);
}