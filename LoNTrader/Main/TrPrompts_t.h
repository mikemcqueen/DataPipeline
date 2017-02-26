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

    TrPrompts_t();

    //
    // DP::Handler_t virtual:
    //

    virtual
    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pMessage);


    //
    // TrSurface_t virtual:
    //

    virtual
    bool
    IsSupportedWindowType(
        Lon::Window::Type_e WindowType) const;

    virtual 
    bool
    TranslateSurface(
        CSurface* pSurface,
        RECT&     rcSurface);

    // Helpers:

    Lon::Window::Type_e
    GetDismissWindowType(
        Lon::Window::Type_e Type) const;

private:

    TrPrompts_t(const TrPrompts_t&);
    TrPrompts_t& operator=(const TrPrompts_t&);

};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRPROMPTS_T_H_

/////////////////////////////////////////////////////////////////////////////
