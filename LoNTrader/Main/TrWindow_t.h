////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrWindow_t.h
//
// Base translator class for LoN Windows.  Performs window validation.
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRWINDOW_T_H
#define Include_TRWINDOW_T_H

#include "TrSurface_t.h"
#include "LonWindow_t.h"
#include "DcrTrades_t.h"

////////////////////////////////////////////////////////////////////////////////

class NoValidatePolicy_t;

template<
    class TranslatePolicy_t,
    class ValidatePolicy_t = NoValidatePolicy_t>
class TrWindow_t :
    public TrSurface_t
{
    // keep this private
    typedef SsTrades_t::ScreenShotData_t ScreenShotData_t;

private:

    Lon::Window::Type_e m_WindowType;
    TranslatePolicy_t& m_TranslatePolicy;
    ValidatePolicy_t&  m_ValidatePolicy;

public:

    TrWindow_t(
        Lon::Window::Type_e WindowType,
        TranslatePolicy_t& TranslatePolicy,
        ValidatePolicy_t&  ValidatePolicy)
        :
        m_WindowType(WindowType),
        m_TranslatePolicy(TranslatePolicy),
        m_ValidatePolicy(ValidatePolicy)
    { }

    ~TrWindow_t() override = default;

    // 
    // DP::Handler_t virtual:
    //

    bool
    Initialize(
        const wchar_t* pszClass) override
    {
        if (!TrSurface_t::Initialize(pszClass))
            return false;
        return m_TranslatePolicy.Initialize() && m_ValidatePolicy.Initialize();
    }

    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override
    {
        if (Validate(pData, m_WindowType))
        {
//            using Lon::Message;
            const ScreenShotData_t*
                pLonData = static_cast<const ScreenShotData_t*>(pData);
//            if (!m_TranslatePolicy.PreTranslate(pLonData))
//                return;
            if (m_ValidatePolicy.Validate(pLonData) &&
                m_TranslatePolicy.Translate(pLonData))
            {
                PostData(0);
                return S_OK;
            }
        }
        return S_FALSE;
    }

private:

    TrWindow_t() = delete;
    TrWindow_t(const TrWindow_t&) = delete;
    TrWindow_t& operator=(const TrWindow_t&) = delete;
};

////////////////////////////////////////////////////////////////////////////////

class OneTablePolicy_t
{
public:

    const Lon::Window::Type_e m_TopWindowType;

private:

    const Lon::Window::Type_e m_DcrWindowType;

    DcrTrades_t& m_Dcr;
    RECT*        m_pRect;

public:

    explicit
    OneTablePolicy_t(
        Lon::Window::Type_e TopWindowType,
        DcrTrades_t&   DcrTrades,
        Lon::Window::Type_e DcrWindowType,
        RECT*          pRect = NULL);

    virtual
    ~OneTablePolicy_t();

    bool
    Initialize()
    {
        return m_Dcr.Initialize();
    }

    bool
    PreTranslate(
        const SsTrades_t::ScreenShotData_t* pData);

    bool
    Translate(
        const SsTrades_t::ScreenShotData_t* pData);

protected:

    virtual
    Lon::Window::Type_e
    GetDcrWindowType() const;

private:

    OneTablePolicy_t() = delete;
    OneTablePolicy_t(const OneTablePolicy_t&) = delete;
    OneTablePolicy_t& operator=(const OneTablePolicy_t&) = delete;
};

////////////////////////////////////////////////////////////////////////////////

template<class GetDcrWindowType_t>
class OneDynamicTablePolicy_t :
    public OneTablePolicy_t
{
    const GetDcrWindowType_t* m_pGetDcrWindowType;

public:

    explicit
    OneDynamicTablePolicy_t(
        Lon::Window::Type_e       TopWindowType,
        DcrTrades_t&              DcrTrades,
        const GetDcrWindowType_t* pGetDcrWindowType,
        RECT*                     pRect = NULL)
        :
        OneTablePolicy_t(
            TopWindowType,
            DcrTrades,
            Lon::Window::Unknown,
            pRect),
        m_pGetDcrWindowType(pGetDcrWindowType)
    { }

protected:

    Lon::Window::Type_e
    GetDcrWindowType() const override
    {
        return m_pGetDcrWindowType->GetDcrWindowType();
    }

private:
    
    OneDynamicTablePolicy_t() = delete;
    OneDynamicTablePolicy_t(const OneDynamicTablePolicy_t&) = delete;
    OneDynamicTablePolicy_t& operator=(const OneDynamicTablePolicy_t&) = delete;
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
        DcrTrades_t&        DcrFirst,
        Lon::Window::Type_e DcrFirstWindowType,
        DcrTrades_t&        DcrSecond,
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

    TwoTablePolicy_t() = delete;
    TwoTablePolicy_t(const TwoTablePolicy_t&) = delete;
    TwoTablePolicy_t& operator=(const TwoTablePolicy_t&) = delete;
};

////////////////////////////////////////////////////////////////////////////////

class NoValidatePolicy_t
{

public:

    NoValidatePolicy_t()
    {
    }

    bool
    Initialize()
    {
        return true;
    }

    bool
    Validate(
        const SsTrades_t::ScreenShotData_t* /*pData*/)
    {
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

#endif  // Include_TRWINDOW_T_H

////////////////////////////////////////////////////////////////////////////////
