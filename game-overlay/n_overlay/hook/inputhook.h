#pragma once

class InputHook : public IHookModule
{
    bool hooked = false;

public:

    bool hook() override;

    void unhook() override;

    void saveInputState();
    void restoreInputState();
};
