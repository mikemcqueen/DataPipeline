////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrWindow.h
//
// Base translate handler class for DCR of a "window" - loosely defined
// as an entity with a unique ID which can be validated.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRWINDOW_H
#define Include_DCRWINDOW_H

#include "DpHandler_t.h"
#include "SsWindow.h"
#include "UiWindowId.h"
//#include "Dcr.h"

class DCR;
using DcrVector_t = std::vector<DCR*>;

namespace DcrWindow
{

using AcquireData_t = SsWindow::Acquire::Data_t;

namespace Translate
{

////////////////////////////////////////////////////////////////////////////////

template<
    class TranslatePolicy_t,
    class ValidatePolicy_t> //  = Policy::NoValidate_t<int, int>>
class Handler_t :
    public DP::Handler_t
{
private:

    Ui::WindowId_t     m_WindowId;
    TranslatePolicy_t& m_TranslatePolicy;
    ValidatePolicy_t&  m_ValidatePolicy;
    wstring            m_name;

public:

    Handler_t(
        Ui::WindowId_t     WindowId,
        TranslatePolicy_t& TranslatePolicy,
        ValidatePolicy_t&  ValidatePolicy,
        const wchar_t*     pName = nullptr)
    :
        m_WindowId(WindowId),
        m_TranslatePolicy(TranslatePolicy),
        m_ValidatePolicy(ValidatePolicy),
        m_name((nullptr != pName) ? pName : L"[unnamed]")
    { }

    #if 1
    ~Handler_t() override
    { }
    #endif

    // 
    // DP::Handler_t virtual:
    //

    bool
    Initialize(
        const wchar_t* pClass) override
    {
        return DP::Handler_t::Initialize(pClass)
            && m_TranslatePolicy.Initialize()
            && m_ValidatePolicy.Initialize();
    }

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pMessage) override
    {
        LogInfo(L"Dcr%s::MessageHandler()", m_name.c_str());
        if (Validate(pMessage, m_WindowId))
        {
            auto& ssData = static_cast<const AcquireData_t&>(*pMessage);
            if (m_ValidatePolicy.Validate(ssData) &&
                m_TranslatePolicy.Translate(ssData))
            {
                PostData(0);
                return S_OK;
            }
        }
        return S_FALSE;
    }

    // 
    // DcrWindow virtual:
    //

    virtual
    void
    PostData(
        DWORD /*Unused*/) = 0;

private:

    bool
    Validate(
        const DP::Message::Data_t* pMessage,
              Ui::WindowId_t       WindowId) const
    {
        if (0 == wcscmp(pMessage->Class, L"SsWindow")) {
            auto& ssData = static_cast<const AcquireData_t&>(*pMessage);
            if (ssData.WindowId == WindowId) {
                return true;
            }
        }
        return false;
    }

private:

    // Explicitly disabled:
    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator=(const Handler_t&);
};

} // Translate

////////////////////////////////////////////////////////////////////////////////

namespace Policy
{

class OneTable_t
{

public:

    const Ui::WindowId_t m_TopWindowId;

private:

    const Ui::WindowId_t m_DcrWindowId;

    DCR&   m_Dcr;
//    RECT*  m_pRect;

public:

    explicit
    OneTable_t(
    Ui::WindowId_t TopWindowId,
    Ui::WindowId_t DcrWindowId,
        DCR&       Dcr);
//,        RECT*      pRect = nullptr);

    ~OneTable_t();

    bool
    Initialize();

    bool
    PreTranslate(
        const AcquireData_t& Data);

    bool
    Translate(
        const AcquireData_t& Data);

private:

    OneTable_t();
    OneTable_t(const OneTable_t&);
    OneTable_t& operator=(const OneTable_t&);
};

////////////////////////////////////////////////////////////////////////////////

class TranslateMany_t
{
public:

//    const Ui::Window::Id_t m_TopWindowId;

private:

//    const Ui::Window::Id_t m_DcrWindowId;

    DcrVector_t& m_DcrVector;

public:

    explicit
    TranslateMany_t(
        DcrVector_t& DcrVector);

    ~TranslateMany_t();

    bool
    Initialize();

    bool
    PreTranslate(
        const AcquireData_t& Data);

    bool
    Translate(
        const AcquireData_t& Data);

private:

    TranslateMany_t();
    TranslateMany_t(const TranslateMany_t&);
    TranslateMany_t& operator=(const TranslateMany_t&);
};

////////////////////////////////////////////////////////////////////////////////

#if PENDING
template<class GetDcrWindowType_t>
class OneDynamicTablePolicy_t :
    public OneTablePolicy_t
{

    const GetDcrWindowType_t* m_pGetDcrWindowType;

public:

    explicit
    OneDynamicTablePolicy_t(
              Lon::Window::Type_e TopWindowType,
              DcrTrades_t&        DcrTrades,
        const GetDcrWindowType_t* pGetDcrWindowType,
              RECT*               pRect = nullptr)
    :
        OneTablePolicy_t(
            TopWindowType,
            DcrTrades,
            Lon::Window::Unknown,
            pRect),
        m_pGetDcrWindowType(pGetDcrWindowType)
    { }

protected:

    virtual
    Lon::Window::Type_e
    GetDcrWindowType() const
    {
        return m_pGetDcrWindowType->GetDcrWindowType();
    }

private:
    
    OneDynamicTablePolicy_t();
    OneDynamicTablePolicy_t(const OneDynamicTablePolicy_t&);
    OneDynamicTablePolicy_t& operator=(const OneDynamicTablePolicy_t&);
};

////////////////////////////////////////////////////////////////////////////////

class TwoTablePolicy_t
{

private:

    DcrTrades_t& m_DcrFirst;
    DcrTrades_t& m_DcrSecond;

    const Lon::Window::Type_e m_TopWindowType;
    const Lon::Window::Type_e m_DcrFirstWindowType;
    const Lon::Window::Type_e m_DcrSecondWindowType;

public:

    explicit
    TwoTablePolicy_t(
        Lon::Window::Type_e TopWindowType,
        DcrTrades_t&   DcrFirst,
        Lon::Window::Type_e DcrFirstWindowType,
        DcrTrades_t&   DcrSecond,
        Lon::Window::Type_e DcrSecondWindowType);

    bool
    Initialize()
    {
        return m_DcrFirst.Initialize() && m_DcrSecond.Initialize();
    }

    bool
    PreTranslate(
        const SsTrades_t::ScreenShotData_t* pData);

    bool
    Translate(
        const SsTrades_t::ScreenShotData_t* pData);

private:

    TwoTablePolicy_t();
    TwoTablePolicy_t(const TwoTablePolicy_t&);
    TwoTablePolicy_t& operator=(const TwoTablePolicy_t&);
};
#endif

class NoValidate_t
{

public:

    NoValidate_t()
    {
    }

    bool
    Initialize()
    {
        return true;
    }

    bool
    Validate(
        const AcquireData_t& /*Data*/)
    {
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
#if PENDING
class ValidateWindowPolicy_t
{

public:

    ValidateWindowPolicy_t();

    bool
    Initialize();

    bool
    Validate(
        const SsTrades_t::ScreenShotData_t* pData);

private:

    bool
    InitAllBitmaps();

    bool
    ValidateSides(
        const CSurface* pSurface,
        const RECT&     rcBounds);

    bool
    ValidateCorners(
        const CSurface* pSurface,
        const RECT&     rcBounds);
};
#endif

////////////////////////////////////////////////////////////////////////////////

} // Policy
} // DcrWindow

#endif  // Include_DCRWINDOW_H

////////////////////////////////////////////////////////////////////////////////
