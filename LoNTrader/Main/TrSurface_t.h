/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrSurface_t.h
//
// Base class translate handler for SurfacePoolItem_t's (e.g. screenshots).
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRSURFACE_T_H_
#define Include_TRSURFACE_T_H_

#include "DpHandler_t.h"
#include "LonMessageTypes.h"
#include "Log.h"
#include "SsTrades_t.h"

/////////////////////////////////////////////////////////////////////////////

class CSurface;

class TrSurface_t :
    public DP::Handler_t
{
    // keep this private
    typedef SsTrades_t::ScreenShotData_t TranslateData_t;

public:

    TrSurface_t() = default;
    ~TrSurface_t() override = default;

    //
    // DP::Handler_t virtual:
    //

/*
    void
    MessageHandler(
        const DP::Message::Data_t *pData) override
    {
        if (!Validate(pData, false))
            return;
    }
*/

    //
    // TrSurface_t virtual:
    //

    virtual
    bool
    IsSupportedWindowType(
        Lon::Window::Type_e /*WindowType*/) const
    {
        return false;
    }

    virtual
    void
    PostData(
        DWORD /*Unused*/)
    {
        LogError(L"TrSurface_t::PostData()");
    }

protected:

    CSurface*
    GetSurface(
        const DP::Message::Data_t* pData) const
    {
        if ((NULL != pData) &&
            (DP::Stage::Acquire == pData->Stage) &&
            (DP::Message::Type::Message == pData->Type) &&
            (Lon::Message::Id::ScreenShot == pData->Id))
        {
            const TranslateData_t*
                pSsData = static_cast<const TranslateData_t*>(pData);
            if (NULL != pSsData->pPoolItem)
            {
                return pSsData->pPoolItem->get();
            }
        }
        throw std::invalid_argument("TrSurface_t::GetSurface(pData)");
    }

    bool
    Validate(
        const DP::Message::Data_t* pMessage,
              Lon::Window::Type_e  WindowType) const
    {
        if (IsScreenShot(pMessage))
        {
            const TranslateData_t*
                pLonData = static_cast<const TranslateData_t*>(pMessage);
            if (pLonData->WindowType == WindowType)
            {
                return LonWindow_t::GetTopWindow().Type == WindowType;
            }
        }
        return false;
    }


    bool
    Validate(
        const DP::Message::Data_t* pMessage)
//              bool                 bValidateWindowType = true)
    {
        bool bValidateWindowType = true;
        if (IsScreenShot(pMessage))
        {
            const TranslateData_t*
                pLonData = static_cast<const TranslateData_t*>(pMessage);
            if (!bValidateWindowType ||
                IsSupportedWindowType(pLonData->WindowType))
            {
                return LonWindow_t::GetTopWindow().Type == pLonData->WindowType;
            }
        }
        return false;
    }

private:

    bool
    IsScreenShot(
        const DP::Message::Data_t* pMessage) const
    {
        if ((NULL != pMessage) &&
            (DP::Stage::Acquire == pMessage->Stage) &&
            (DP::Message::Type::Message == pMessage->Type) &&
            (Lon::Message::Id::ScreenShot == pMessage->Id))
        {
            //TODO: && (static_cast<screenshot_t*>(pMessage)->pPoolItem != NULL)  - ?
            return true;
        }
        return false;
    }

};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRSURFACE_T_H_

/////////////////////////////////////////////////////////////////////////////
