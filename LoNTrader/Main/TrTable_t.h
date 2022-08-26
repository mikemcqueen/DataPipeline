/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrTable_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRTABLE_T_H_
#define Include_TRTABLE_T_H_

/////////////////////////////////////////////////////////////////////////////

#include "TrWindow_t.h"
#include "LonWindow_t.h"
#include "DdUtil.h"

/////////////////////////////////////////////////////////////////////////////

class TrTable_t :
    public TrWindow_t
{

public:

    TrTable_t() = default;
    virtual ~TrTable_t() = default;

    //
    // DP::TranslateHandler_t virtual:
    //

    void
    Translate(
        const DP::AcquireData_t* pData) override;
    {
        if (!Validate(pData))
            return;

        CSurface* pSurface = GetSurface(pData);
        RECT rc = { 0 };
        if (PreTranslateSurface(pSurface, rc) &&
            TranslateSurface(pSurface, rc))
        {
            PostData(pData->Id);
        }
    }

    //
    // TrSurface_t virtual:
    //

    bool
    IsSupportedWindowType(
        const LonWindow_t::Type_e /*WindowType*/) const override
    {
        return true;
    }

    void
    PostData(
        DWORD /*AcquireId*/) override
    {
        LogError(L"TrTable_t::PostData();");
    }


    //
    // TrTable_t virtual:
    //

    virtual
    bool
    PreTranslateSurface(
        CSurface* /*pSurface*/,
        RECT&     /*rcSurface*/)
    {
        return true; // !IsRectEmpty(&rcSurface);
    }

    virtual
    bool
    TranslateSurface(
        CSurface* pSurface,
        RECT& rcSurface) = 0;

protected:

    CSurface*
    GetSurface(
        const DP::AcquireData_t* pData) const
    {
        return reinterpret_cast<const DP::AcquireSurfacePoolItem_t*>(pData)->pPoolItem->get();
    }

    bool
    Validate(
        const DP::AcquireData_t*  pData,
        const LonWindow_t::Type_e WindowType) const
    {
        if (DP::SurfacePoolItem != pData->Format)
            return false;
        SsTrades_t::AcquireData_t*
            pLonData = (SsTrades_t::AcquireData_t*)pData;
        return WindowType == pLonData->WindowType;
    }

    bool
    Validate(
        const DP::AcquireData_t*  pData) const
    {
        if (DP::SurfacePoolItem != pData->Format)
            return false;
        return IsSupportedWindowType(reinterpret_cast<const SsTrades_t::AcquireData_t*>(pData)->WindowType);
    }
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TrTable_T_H_

/////////////////////////////////////////////////////////////////////////////
