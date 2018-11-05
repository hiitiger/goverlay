#pragma once


class InputHook : public IHookModule
{
    std::atomic<bool> hooked_ = false;

public:

    bool hooked() const { return hooked_; }

    bool hook() override;
    void unhook() override;

    void saveInputState();
    void restoreInputState();

};


namespace Windows
{
    namespace OrginalApi
    {
        SHORT GetAsyncKeyState(_In_ int vKey);
        SHORT GetKeyState(_In_ int vKey);
        BOOL GetKeyboardState(__out_ecount(256) PBYTE lpKeyState);

        INT  ShowCursor(__in BOOL bShow);
        BOOL  GetCursorPos(LPPOINT lpPoint);
        BOOL  SetCursorPos(int X, int Y);

        HCURSOR GetCursor();
        HCURSOR SetCursor(HCURSOR cursor);

        UINT GetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
        UINT GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
    }
}