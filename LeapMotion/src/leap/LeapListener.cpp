#include "leap/LeapListener.hpp"
#include "Utils.hpp"

static const char *fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
static const char *boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };
static const char *stateNames[] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };

Leap::HandList handList;

void LeapListener::onInit(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Initialized");
}

void LeapListener::onConnect(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Device Connected");
    controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
    controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
}

void LeapListener::onDisconnect(const Leap::Controller& controller) 
{
    // Note: not dispatched when running in a debugger.
    LOG_MESSAGE("[LeapMotion] Device Disconnected");
}

void LeapListener::onExit(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Exited");
}

void LeapListener::onFrame(const Leap::Controller& controller) 
{
    // Get the most recent frame and report some basic information
    const Leap::Frame frame = controller.frame();

    std::stringstream frameInfo;   
    frameInfo << "Frame id: " << frame.id() << ", timestamp: " << frame.timestamp()
              << ", hands: " << frame.hands().count()
              << ", extended fingers: " << frame.fingers().extended().count()
              << ", tools: " << frame.tools().count()
              << ", gestures: " << frame.gestures().count();

    LOG_MESSAGE(frameInfo.str());

    Leap::HandList hands = frame.hands();
    handList = hands;

    for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) 
    {
        // Get the first hand
        const Leap::Hand hand = *hl;
        // Get the Arm bone
        Leap::Arm arm = hand.arm();
        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";

        // Get the hand's normal vector and direction
        const Leap::Vector normal = hand.palmNormal();
        const Leap::Vector direction = hand.direction();

        char palmInfo[128];
        sprintf(palmInfo, "%s, id: %d, palm position: [%f, %f, %f]", handType.c_str(), 
                                                                     hand.id(), 
                                                                     hand.palmPosition().x, hand.palmPosition().y, hand.palmPosition().z);
        LOG_MESSAGE(palmInfo);

        // Calculate the hand's pitch, roll, and yaw angles
        char handInfo[128];
        sprintf(handInfo, "pitch: %f, degrees, roll: %f, degrees, yaw %f degrees", direction.pitch() * Leap::RAD_TO_DEG,
                                                                                   normal.roll() * Leap::RAD_TO_DEG,
                                                                                   direction.yaw() * Leap::RAD_TO_DEG);
        LOG_MESSAGE(handInfo);

        char armInfo[256];
        sprintf(armInfo, "Arm direction: [%f, %f, %f], wrist position: [%f, %f, %f], elbow position: [%f, %f, %f]", arm.direction().x, arm.direction().y, arm.direction().z,
                                                                                                                    arm.wristPosition().x, arm.wristPosition().y, arm.wristPosition().z,
                                                                                                                    arm.elbowPosition().x, arm.elbowPosition().y, arm.elbowPosition().z);
        LOG_MESSAGE(armInfo);

        // Get fingers
        const Leap::FingerList fingers = hand.fingers();
        for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) 
        {
            const Leap::Finger finger = *fl;

            char fingerInfo[128];
            sprintf(fingerInfo, "%s finger, id: %d, length: %fmm, width: %f", fingerNames[finger.type()],
                                                                              finger.id(),
                                                                              finger.length(),
                                                                              finger.width());
            LOG_MESSAGE(fingerInfo);

            // Get finger bones
            for (int b = 0; b < 4; ++b) 
            {
                Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
                Leap::Bone bone = finger.bone(boneType);

                char boneInfo[256];
                sprintf(boneInfo, "%s bone, start: [%f, %f, %f], end: [%f, %f, %f], direction: [%f, %f, %f]", boneNames[boneType], 
                                                                                                              bone.prevJoint().x, bone.prevJoint().y, bone.prevJoint().z,
                                                                                                              bone.nextJoint().x, bone.nextJoint().y, bone.nextJoint().z,
                                                                                                              bone.direction().x, bone.direction().y, bone.direction().z);
                LOG_MESSAGE(boneInfo);
            }
        }
    }

    // Get tools
    const Leap::ToolList tools = frame.tools();
    for (Leap::ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) 
    {
        const Leap::Tool tool = *tl;

        char toolInfo[128];
        sprintf(toolInfo, "Tool, id: %d, position: [%f, %f, %f], direction: [%f, %f, %f]", tool.id(),
                                                                                           tool.tipPosition().x, tool.tipPosition().y, tool.tipPosition().z,
                                                                                           tool.direction().x, tool.direction().y, tool.direction().z);
        LOG_MESSAGE(toolInfo);
    }

    // Get gestures
    const Leap::GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) 
    {
        Leap::Gesture gesture = gestures[g];

        switch (gesture.type()) 
        {
        case Leap::Gesture::TYPE_CIRCLE:
        {
            Leap::CircleGesture circle = gesture;
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
            sprintf(msg, "Circle id: %d, state: %s, progress %f, radius: %f, angle %f, %s", gesture.id(), 
                                                                                            stateNames[gesture.state()], 
                                                                                            circle.progress(),
                                                                                            circle.radius(),
                                                                                            sweptAngle * Leap::RAD_TO_DEG, 
                                                                                            clockwiseness.c_str());
            LOG_MESSAGE(msg);
            break;
        }
        case Leap::Gesture::TYPE_SWIPE:
        {
            Leap::SwipeGesture swipe = gesture;

            char msg[256];
            sprintf(msg, "Swipe id: %d, state: %s, direction: [%f, %f, %f], speed %f", gesture.id(), 
                                                                                       stateNames[gesture.state()], 
                                                                                       swipe.direction().x, swipe.direction().y, swipe.direction().z,
                                                                                       swipe.speed());
            LOG_MESSAGE(msg);
            break;
        }
        case Leap::Gesture::TYPE_KEY_TAP:
        {
            Leap::KeyTapGesture tap = gesture;

            char msg[256];
            sprintf(msg, "Key Tap id: %d, state %s, position: [%f, %f, %f], direction [%f, %f, %f]", gesture.id(), 
                                                                                                    stateNames[gesture.state()], 
                                                                                                    tap.position().x, tap.position().y, tap.position().z, 
                                                                                                    tap.direction().x, tap.direction().y, tap.direction().z);
            LOG_MESSAGE(msg);
            break;
        }
        case Leap::Gesture::TYPE_SCREEN_TAP:
        {
            Leap::ScreenTapGesture screentap = gesture;

            char msg[256];
            sprintf(msg, "Key Tap id: %d, state %s, position: [%f, %f, %f], direction [%f, %f, %f]", gesture.id(),
                                                                                                     stateNames[gesture.state()],
                                                                                                     screentap.position().x, screentap.position().y, screentap.position().z,
                                                                                                     screentap.direction().x, screentap.direction().y, screentap.direction().z);
            LOG_MESSAGE(msg);
            break;
        }
        default:
            LOG_MESSAGE_ASSERT(false, "Unknown gesture type.");
            break;
        }
    }
}

void LeapListener::onFocusGained(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Focus Gained");
}

void LeapListener::onFocusLost(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Focus Lost");
}

void LeapListener::onDeviceChange(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Device Changed");
    const Leap::DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i) 
    {
        char buff[128];
        sprintf(buff, "id: %s, isStreaming: %s", devices[i].toString().c_str(), devices[i].isStreaming() ? "true" : "false");
        LOG_MESSAGE(buff);
    }
}

void LeapListener::onServiceConnect(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Service Connected");
}

void LeapListener::onServiceDisconnect(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Service Disconnected");
}