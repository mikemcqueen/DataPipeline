/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// UiInput.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_UIINPUT_H
#define Include_UIINPUT_H

#include "UiTypes.h"

struct Rect_t;

namespace Ui
{

class Input_t
{

private:

    static const BYTE   vkInvalid             = 0xff;
    static const size_t DefaultMoveDelay      = 100;
    static const size_t DefaultClickDelay     = 100;
    static const size_t DefaultSendCharDelay  = 20;
    static const size_t DefaultMoveClickDelay = 250;

public:

    Input_t();

    static
    bool
    Click(
        HWND            hWnd,
        POINT           point,
        Mouse::Button_t Button = Mouse::Button::Left);

    static
    bool
    Click(
        Mouse::Button_t Button = Mouse::Button::Left);

    static 
    bool
    SendChars(
        const wchar_t* pText);

    static 
    bool
    SendChar(
        wchar_t ch,
        size_t  count = 1,
        size_t delay = DefaultSendCharDelay);

    static
    bool
    SendKey(
        WORD vkKey,
        bool keyUp = false);

    static
    bool
    MoveToAbsolute(
        HWND hWnd,
        POINT pt,
        size_t count = 1);

    static
    bool
    MouseDown(
        Mouse::Button_t Button);

    static
    bool
    MouseUp(
        Mouse::Button_t Button);

    static 
    bool
    Send(
        INPUT& input);

private:

    static 
    void
    Pause(
        size_t Delay)
    {
        if (0 != Delay)
        {
            ::Sleep(Delay);
        }
    }

    static
    void
    Normalize(
        POINT& Point);

    static
    short
    GetVkCode(
        wchar_t ch);

};

} // Ui

#endif // Include_UIINPUT_H
