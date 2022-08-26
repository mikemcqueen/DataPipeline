/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrPrompts_t.h
//
// Translate handler for prompts and system message windows.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRPROMPTS_T_H_
#define Include_TRPROMPTS_T_H_

#include "TrSurface_t.h"
#include "LonWindow_t.h"

/////////////////////////////////////////////////////////////////////////////

class Charset_t;
class TextTable_i;

class TrPrompts_t :
    public TrSurface_t
{

public:

    TrPrompts_t() = default;

    //
    // DP::Handler_t virtual:
    //

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pMessage) override;


    //
    // TrSurface_t virtual:
    //

    bool
    IsSupportedWindowType(
        Lon::Window::Type_e WindowType) const override;

    //

    Lon::Window::Type_e
    GetDismissWindowType(
        Lon::Window::Type_e Type) const;

private:

    TrPrompts_t(const TrPrompts_t&) = delete;
    TrPrompts_t& operator=(const TrPrompts_t&) = delete;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRPROMPTS_T_H_

/////////////////////////////////////////////////////////////////////////////
