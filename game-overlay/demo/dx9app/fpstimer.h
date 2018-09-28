#pragma once

class FpsTimer
{
public:
    FpsTimer();

    void start();
    float tick();

    int fps();

private:
    int m_frames;
    DWORD m_lastTickCount;
    int m_fps;
};