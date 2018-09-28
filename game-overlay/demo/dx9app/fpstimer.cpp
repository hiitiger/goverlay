#include "stdafx.h"

#include "fpstimer.h"

FpsTimer::FpsTimer()
    : m_frames(0)
    , m_lastTickCount(0)
    , m_fps(0)
{

}

void FpsTimer::start()
{
    m_lastTickCount = GetTickCount();
}

float FpsTimer::tick()
{
    m_frames += 1;

    DWORD cur = GetTickCount();
    float delta = (float)(cur - m_lastTickCount);

    if (delta> 1000)
    {
        m_fps = (int)((float)m_frames * 1000.0 /(float)(delta));
        m_lastTickCount = cur;
        m_frames = 0;
    }

    return delta;
}

int FpsTimer::fps()
{
    return m_fps;
}
