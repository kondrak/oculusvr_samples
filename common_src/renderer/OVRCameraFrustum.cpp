#include "renderer/OVRCameraFrustum.hpp"
#include "renderer/ShaderManager.hpp"
#include "Extras/OVR_Math.h"

OVRCameraFrustum::~OVRCameraFrustum()
{
    if (glIsBuffer(m_vertexBuffers[0]))
    {
        glDeleteBuffers(3, m_vertexBuffers);
    }

    if (glIsVertexArray(m_vertexArray))
    {
        glDeleteVertexArrays(1, &m_vertexArray);
    }
}

void OVRCameraFrustum::Recalculate(ovrHmd hmd)
{
    ovrTrackingState tState = ovr_GetTrackingState(hmd, 0.0f);
    ovrHmdDesc hmdDesc      = ovr_GetHmdDesc(hmd);
    ovrVector3f trackerPose = tState.CameraPose.Position;

    float trackerFar  = hmdDesc.CameraFrustumFarZInMeters;
    float trackerNear = hmdDesc.CameraFrustumNearZInMeters;
    float trackerHFov = hmdDesc.CameraFrustumHFovInRadians;
    float trackerVFov = hmdDesc.CameraFrustumVFovInRadians;

    float hScale = tanf(trackerHFov / 2.f);
    float vScale = tanf(trackerVFov / 2.f);

    // camera orientation quaternion
    OVR::Quatf trackerOrientationQuat(tState.CameraPose.Orientation.x,
                                      tState.CameraPose.Orientation.y,
                                      tState.CameraPose.Orientation.z,
                                      tState.CameraPose.Orientation.w);

    // orientation indicator vector running from camera pose to near plane
    OVR::Vector3f trackerOrientationVec(0.f, 0.f, trackerNear);

    // near plane vertex positions
    OVR::Vector3f nearV1(-hScale * trackerNear, vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV2(-hScale * trackerNear, -vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV3(hScale * trackerNear, -vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV4(hScale * trackerNear, vScale * trackerNear, trackerNear);

    // far plane vertex positions
    OVR::Vector3f farV1(-hScale * trackerFar, vScale * trackerFar, trackerFar);
    OVR::Vector3f farV2(-hScale * trackerFar, -vScale * trackerFar, trackerFar);
    OVR::Vector3f farV3(hScale * trackerFar, -vScale * trackerFar, trackerFar);
    OVR::Vector3f farV4(hScale * trackerFar, vScale * trackerFar, trackerFar);

    // reorient all vectors by current tracker camera orientation
    trackerOrientationVec = trackerOrientationQuat.Rotate(trackerOrientationVec);

    nearV1 = trackerOrientationQuat.Rotate(nearV1);
    nearV2 = trackerOrientationQuat.Rotate(nearV2);
    nearV3 = trackerOrientationQuat.Rotate(nearV3);
    nearV4 = trackerOrientationQuat.Rotate(nearV4);

    farV1 = trackerOrientationQuat.Rotate(farV1);
    farV2 = trackerOrientationQuat.Rotate(farV2);
    farV3 = trackerOrientationQuat.Rotate(farV3);
    farV4 = trackerOrientationQuat.Rotate(farV4);

    OVR::Vector3f orientationVector(trackerPose.x + trackerOrientationVec.x,
        trackerPose.y + trackerOrientationVec.y,
        trackerPose.z + trackerOrientationVec.z);
    // tracker camera frustum
    const GLfloat frustumVertexData[] = {
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV1.x, trackerPose.y + farV1.y, trackerPose.z + farV1.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV2.x, trackerPose.y + farV2.y, trackerPose.z + farV2.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV3.x, trackerPose.y + farV3.y, trackerPose.z + farV3.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV4.x, trackerPose.y + farV4.y, trackerPose.z + farV4.z,
        // orientation vector (trackerPose to near plane)
        trackerPose.x, trackerPose.y, trackerPose.z,
        orientationVector.x, orientationVector.y, orientationVector.z
    };

    // near plane of the tracking camera
    const GLfloat nearPlaneVertexData[] = {
        trackerPose.x + nearV1.x, trackerPose.y + nearV1.y, trackerPose.z + nearV1.z,
        trackerPose.x + nearV2.x, trackerPose.y + nearV2.y, trackerPose.z + nearV2.z,
        trackerPose.x + nearV3.x, trackerPose.y + nearV3.y, trackerPose.z + nearV3.z,
        trackerPose.x + nearV4.x, trackerPose.y + nearV4.y, trackerPose.z + nearV4.z
    };

    // far plane of the tracking camera
    const GLfloat farPlaneVertexData[] = {
        trackerPose.x + farV1.x, trackerPose.y + farV1.y, trackerPose.z + farV1.z,
        trackerPose.x + farV2.x, trackerPose.y + farV2.y, trackerPose.z + farV2.z,
        trackerPose.x + farV3.x, trackerPose.y + farV3.y, trackerPose.z + farV3.z,
        trackerPose.x + farV4.x, trackerPose.y + farV4.y, trackerPose.z + farV4.z
    };

    if (glIsBuffer(m_vertexBuffers[0]))
        glDeleteBuffers(3, m_vertexBuffers);

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);

    // create line VAO
    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(3, m_vertexBuffers);

    glBindVertexArray(m_vertexArray);

    // Get a handle for our buffers (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frustumVertexData), frustumVertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nearPlaneVertexData), nearPlaneVertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(farPlaneVertexData), farPlaneVertexData, GL_STATIC_DRAW);
}

void OVRCameraFrustum::OnRender()
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::OVRFrustumShader);
    GLuint vertexPositionAttr = glGetAttribLocation(shader.id, "inVertex");

    const float frustumColor[3] = { 0.67f, 0.27f, 0.05f };
    const float planeColor[3] = { 1.f, 0.f, 0.f };
    const float orientationVecColor[3] = { 0.f, 1.f, 0.f };

    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render camera bounds
    glBindVertexArray(m_vertexArray);
    glEnableVertexAttribArray(vertexPositionAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[0]);
    glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // draw the frustum lines
    glUniform3fv(shader.uniforms[VertexColor], 1, frustumColor);
    glDrawArrays(GL_LINES, 0, 8);

    // draw the orientation vector with separate color
    glUniform3fv(shader.uniforms[VertexColor], 1, orientationVecColor);
    glDrawArrays(GL_LINES, 8, 2);

    glDisableVertexAttribArray(vertexPositionAttr);

    // render near and far planes
    glUniform3fv(shader.uniforms[VertexColor], 1, planeColor);

    for (int i = 1; i < 3; i++)
    {
        glEnableVertexAttribArray(vertexPositionAttr);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_LINE_LOOP, 0, 4);

        glDisableVertexAttribArray(vertexPositionAttr);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
}
