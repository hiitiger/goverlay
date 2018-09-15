#pragma once
class GameTimer
{
public:
    GameTimer();
    ~GameTimer();

    float GameTime()const; // in seconds, do not contain paused time
    float DeltaTime()const; // in seconds

    void Reset(); // Call before message loop.
    void Start(); // Call when unpaused.
    void Stop(); // Call when paused.
    void Tick(); // Call every frame.

private:
    double mSecondsPerCount;
    double mDeltaTime;

    __int64 mBaseTime; //game start time, stat constant during the game.
    __int64 mPausedTime;
    __int64 mStopTime;

    __int64 mPrevTime;
    __int64 mCurrTime;

    bool mStopped;
};

