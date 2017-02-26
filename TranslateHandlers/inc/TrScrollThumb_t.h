/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrScrollThumb_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRSCROLLTHUMB_T_H_
#define Include_TRSCROLLTHUMB_T_H_

#include "DpHandler_t.h"
#include "UiTypes.h"

/////////////////////////////////////////////////////////////////////////////

class TrScrollThumb_t :
    public DP::Handler_t
{

private:

    const Ui::Window_t& m_MainWindow;

public:

    TrScrollThumb_t(
        const Ui::Window_t& MainWindow);

    ~TrScrollThumb_t();

    //
    // DP::Handler_t virtual:
    //

    virtual
    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pMessage);

private:

    TrScrollThumb_t();
    TrScrollThumb_t(const TrScrollThumb_t&);
    TrScrollThumb_t& operator=(const TrScrollThumb_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TrScrollThumb_t_H

/////////////////////////////////////////////////////////////////////////////

