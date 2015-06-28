#include "leap/LeapListener.hpp"
#include "leap/LeapLogger.hpp"

#ifdef _DEBUG
#define LEAP_DEBUG_ENABLED
#endif

void LeapListener::onInit(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Initialized");
}

void LeapListener::onConnect(const Leap::Controller& controller) 
{
    LOG_MESSAGE("[LeapMotion] Device Connected");

    // track all the gestures
    controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
    controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Leap::Gesture::TYPE_SWIPE);

    // allow processing images + optimize for HMD
    controller.setPolicy(static_cast<Leap::Controller::PolicyFlag>(Leap::Controller::POLICY_IMAGES | Leap::Controller::POLICY_OPTIMIZE_HMD));
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

#ifdef LEAP_DEBUG_ENABLED
    debugFrameInfo(frame);
#endif

    Leap::HandList hands = frame.hands();

    for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) 
    {
        // Get the first hand
        const Leap::Hand hand = *hl;
        // Get the Arm bone
        Leap::Arm arm = hand.arm();

#ifdef LEAP_DEBUG_ENABLED
        debugHandInfo(hand);
        debugArmInfo(arm);
#endif

        // Get fingers
        const Leap::FingerList fingers = hand.fingers();
        for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) 
        {
            const Leap::Finger finger = *fl;

#ifdef LEAP_DEBUG_ENABLED
            debugFingerInfo(finger);
#endif
            // Get finger bones
            for (int b = 0; b < 4; ++b) 
            {
                Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
                Leap::Bone bone = finger.bone(boneType);
                
#ifdef LEAP_DEBUG_ENABLED
                debugBoneInfo(bone, boneType);
#endif
            }
        }
    }

    // Get tools
    const Leap::ToolList tools = frame.tools();
    for (Leap::ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) 
    {
        const Leap::Tool tool = *tl;

#ifdef LEAP_DEBUG_ENABLED
        debugToolInfo(tool);
#endif
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
            
#ifdef LEAP_DEBUG_ENABLED
            debugCircleGestureInfo(circle, controller);
#endif
            break;
        }
        case Leap::Gesture::TYPE_SWIPE:
        {
            Leap::SwipeGesture swipe = gesture;

#ifdef LEAP_DEBUG_ENABLED
            debugSwipeGestureInfo(swipe);
#endif
            break;
        }
        case Leap::Gesture::TYPE_KEY_TAP:
        {
            Leap::KeyTapGesture tap = gesture;

#ifdef LEAP_DEBUG_ENABLED
            debugKeyTapGestureInfo(tap);
#endif
            break;
        }
        case Leap::Gesture::TYPE_SCREEN_TAP:
        {
            Leap::ScreenTapGesture screentap = gesture;

#ifdef LEAP_DEBUG_ENABLED
            debugScreenTapGestureInfo(screentap);
#endif
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
#ifdef LEAP_DEBUG_ENABLED
        debugDeviceInfo(devices[i]);
#endif
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