#ifndef LEAPLOGGER_HPP
#define LEAPLOGGER_HPP
#include "Leap.h"
#include "Utils.hpp"

/*
 * Leap Motion debug helper functions
 * These functions print out the same data type as ones found in Leap Motion SDK sample
 */


static const char *fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
static const char *boneNames[]   = { "Metacarpal", "Proximal", "Middle", "Distal" };
static const char *stateNames[]  = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };


void debugFrameInfo(const Leap::Frame &frame)
{
    std::stringstream frameInfo;
    frameInfo << "Frame id: " << frame.id() << ", timestamp: " << frame.timestamp()
              << ", hands: " << frame.hands().count()
              << ", extended fingers: " << frame.fingers().extended().count()
              << ", tools: " << frame.tools().count()
              << ", gestures: " << frame.gestures().count();

    LOG_MESSAGE(frameInfo.str());
}

void debugHandInfo(const Leap::Hand &hand)
{
    const char *handType = hand.isLeft() ? "Left hand" : "Right hand";

    // Get the hand's normal vector and direction
    const Leap::Vector normal    = hand.palmNormal();
    const Leap::Vector direction = hand.direction();

    char palmInfo[128];
    sprintf(palmInfo, "%s, id: %d, palm position: [%f, %f, %f]", handType, hand.id(),
                                                                 hand.palmPosition().x, hand.palmPosition().y, hand.palmPosition().z);
    LOG_MESSAGE(palmInfo);

    char handInfo[128];
    sprintf(handInfo, "pitch: %f, degrees, roll: %f, degrees, yaw %f degrees", direction.pitch() * Leap::RAD_TO_DEG,
                                                                               normal.roll() * Leap::RAD_TO_DEG,
                                                                               direction.yaw() * Leap::RAD_TO_DEG);
    LOG_MESSAGE(handInfo);
}

void debugArmInfo(const Leap::Arm &arm)
{
    char armInfo[256];
    sprintf(armInfo, "Arm direction: [%f, %f, %f], wrist position: [%f, %f, %f], elbow position: [%f, %f, %f]", arm.direction().x, arm.direction().y, arm.direction().z,
                                                                                                                arm.wristPosition().x, arm.wristPosition().y, arm.wristPosition().z,
                                                                                                                arm.elbowPosition().x, arm.elbowPosition().y, arm.elbowPosition().z);
    LOG_MESSAGE(armInfo);
}

void debugFingerInfo(const Leap::Finger &finger)
{
    char fingerInfo[128];
    sprintf(fingerInfo, "%s finger, id: %d, length: %fmm, width: %f", fingerNames[finger.type()],
                                                                      finger.id(),
                                                                      finger.length(),
                                                                      finger.width());
    LOG_MESSAGE(fingerInfo);
}

void debugBoneInfo(const Leap::Bone &bone, Leap::Bone::Type boneType)
{
    char boneInfo[256];
    sprintf(boneInfo, "%s bone, start: [%f, %f, %f], end: [%f, %f, %f], direction: [%f, %f, %f]", boneNames[boneType],
                                                                                                  bone.prevJoint().x, bone.prevJoint().y, bone.prevJoint().z,
                                                                                                  bone.nextJoint().x, bone.nextJoint().y, bone.nextJoint().z,
                                                                                                  bone.direction().x, bone.direction().y, bone.direction().z);
    LOG_MESSAGE(boneInfo);
}

void debugToolInfo(const Leap::Tool &tool)
{
    char toolInfo[128];
    sprintf(toolInfo, "Tool, id: %d, position: [%f, %f, %f], direction: [%f, %f, %f]", tool.id(),
                                                                                       tool.tipPosition().x, tool.tipPosition().y, tool.tipPosition().z,
                                                                                       tool.direction().x, tool.direction().y, tool.direction().z);
    LOG_MESSAGE(toolInfo);
}

void debugCircleGestureInfo(const Leap::CircleGesture &circle, const Leap::Controller &controller)
{
    std::string clockwiseness;

    if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI / 2)
    {
        clockwiseness = "CW";
    }
    else
    {
        clockwiseness = "CCW";
    }

    // Calculate angle swept since last frame
    float sweptAngle = 0;
    if (circle.state() != Leap::Gesture::STATE_START)
    {
        Leap::CircleGesture previousUpdate = Leap::CircleGesture(controller.frame(1).gesture(circle.id()));
        sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * Leap::PI;
    }

    char msg[256];
    sprintf(msg, "Circle id: %d, state: %s, progress %f, radius: %f, angle %f, %s", circle.id(),
                                                                                    stateNames[circle.state()],
                                                                                    circle.progress(),
                                                                                    circle.radius(),
                                                                                    sweptAngle * Leap::RAD_TO_DEG,
                                                                                    clockwiseness.c_str());
    LOG_MESSAGE(msg);
}

void debugSwipeGestureInfo(const Leap::SwipeGesture &swipe)
{
    char msg[256];
    sprintf(msg, "Swipe id: %d, state: %s, direction: [%f, %f, %f], speed %f", swipe.id(),
                                                                               stateNames[swipe.state()],
                                                                               swipe.direction().x, swipe.direction().y, swipe.direction().z,
                                                                               swipe.speed());
    LOG_MESSAGE(msg);
}

void debugKeyTapGestureInfo(const Leap::KeyTapGesture &tap)
{
    char msg[256];
    sprintf(msg, "Key Tap id: %d, state %s, position: [%f, %f, %f], direction [%f, %f, %f]", tap.id(),
                                                                                             stateNames[tap.state()],
                                                                                             tap.position().x, tap.position().y, tap.position().z,
                                                                                             tap.direction().x, tap.direction().y, tap.direction().z);
    LOG_MESSAGE(msg);
}

void debugScreenTapGestureInfo(const Leap::ScreenTapGesture &screentap)
{
    char msg[256];
    sprintf(msg, "Key Tap id: %d, state %s, position: [%f, %f, %f], direction [%f, %f, %f]", screentap.id(),
                                                                                             stateNames[screentap.state()],
                                                                                             screentap.position().x, screentap.position().y, screentap.position().z,
                                                                                             screentap.direction().x, screentap.direction().y, screentap.direction().z);
    LOG_MESSAGE(msg);
}

void debugDeviceInfo(const Leap::Device &device)
{
    char buff[128];
    sprintf(buff, "id: %s, isStreaming: %s", device.toString().c_str(), device.isStreaming() ? "true" : "false");
    LOG_MESSAGE(buff);
}

#endif