#include "leap/LeapMotion.hpp"
#include "leap/LeapListener.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/ShaderManager.hpp"

GLuint m_vertexArray;
//GLuint m_vertexBuffers[3];

GLuint m_fingerVertexBuffers[40];

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
    if (!handList.count())
        return;

  //  Leap::Hand hand = handList[0];

    //Leap::FingerList fingers = hand.fingers();

   // Leap::Finger f = hand.fingers()[0];

    //const Leap::Bone &bone = f.bone(static_cast<Leap::Bone::Type>(1));


  //  Leap::Vector trackerPose(bone.nextJoint().x, bone.nextJoint().y, bone.nextJoint().z);
   // Leap::Vector farV1(bone.prevJoint().x, bone.prevJoint().y, bone.prevJoint().z);

  //  trackerPose *= 0.01f;
  //  farV1 *= 0.01f;

    // tracker camera frustum
  /*  GLfloat frustumVertexData[] = {
        trackerPose.x, trackerPose.y, trackerPose.z,
        farV1.x, farV1.y, farV1.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV2.x, trackerPose.y + farV2.y, trackerPose.z + farV2.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV3.x, trackerPose.y + farV3.y, trackerPose.z + farV3.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV4.x, trackerPose.y + farV4.y, trackerPose.z + farV4.z, 
    };
*/

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);

    // create line VAO
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

    for (int i = 0; i < handList.count(); i++)
    {
        Leap::Hand hand = handList[i];
        Leap::FingerList fingers = hand.fingers();
        for (int j = 0; j < fingers.count(); j++)
        {
            const Leap::Finger &finger = fingers[j];

            for (int k = Leap::Bone::TYPE_METACARPAL; k <= Leap::Bone::TYPE_DISTAL; k++)
            {
                const Leap::Bone &bone = finger.bone(static_cast<Leap::Bone::Type>(k));

                // if (j == Leap::Bone::Type::TYPE_METACARPAL)
                //     continue;

                GLfloat fingerVertexData[] = { bone.prevJoint().x, bone.prevJoint().y, bone.prevJoint().z,
                    bone.nextJoint().x, bone.nextJoint().y, bone.nextJoint().z };

                if (glIsBuffer(m_fingerVertexBuffers[k + j * 4 + i * 20]))
                    glDeleteBuffers(1, &m_fingerVertexBuffers[k + j * 4 + i * 20]);

                glGenBuffers(1, &m_fingerVertexBuffers[k + j * 4  + i * 20]);

                // Get a handle for our buffers (VBO)
                glBindBuffer(GL_ARRAY_BUFFER, m_fingerVertexBuffers[k + j * 4 + i * 20]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(fingerVertexData), fingerVertexData, GL_STATIC_DRAW);
            }
        }
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

    glDisableVertexAttribArray(vertexPositionAttr);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
}

void LeapMotion::Destroy()
{
    m_leapData->m_controller.removeListener(m_leapData->m_listener);
}