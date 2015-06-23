#include "leap/LeapMotion.hpp"
#include "leap/LeapListener.hpp"

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


void LeapMotion::Destroy()
{
    m_leapData->m_controller.removeListener(m_leapData->m_listener);
}