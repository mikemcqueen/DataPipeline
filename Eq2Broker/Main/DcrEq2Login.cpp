////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DcrEq2Login.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Eq2Login.h"
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "DcrBase_t.h"

namespace Broker
{
namespace Eq2Login
{
namespace Translate
{

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Window::ManagerBase_t& windowManager)
:
    HandlerBase_t(
        TopWindowId,
        m_TranslatePolicy,
        m_ValidatePolicy,
        L"Eq2Login"),
    m_TranslatePolicy(m_DcrVector),
    m_dcrCharacterEdit(
        windowManager.GetWindow(),
        Widget::Id::CharacterEdit,
        DcrBase_t::GetCharset()),
    m_dcrServerEdit(
        windowManager.GetWindow(),
        Widget::Id::ServerEdit,
        DcrBase_t::GetCharset()),
    m_windowManager(windowManager)
{
    m_DcrVector.push_back(&m_dcrCharacterEdit);
    m_DcrVector.push_back(&m_dcrServerEdit);
    for (size_t button = 0; button < kCharacterButtonCount; ++button)
    {
        shared_ptr<DcrRect_t> spDcrRect(
            new DcrRect_t(
                windowManager.GetWindow(),
                Widget::Id::FirstCharacterButton + button,
                DcrBase_t::GetCharset(),
                false,
                BkHighColor));
        m_DcrVector.push_back(spDcrRect.get());
        m_dcrCharacterButtons.push_back(spDcrRect);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*Unused*/)
{
    extern bool g_noDcrPost;
    if (g_noDcrPost)
    {
        return;
    }
    void *pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (NULL == pBuffer)
    {
        LogError(L"DcrEq2Login::PostData(): Alloc callback data failed.");
    }
    else
    {
        LogInfo(L"DcrEq2Login::PostData()");
        Data_t::CharacterButton_t characterButtons[kCharacterButtonCount];
        for (size_t button = 0; button < m_dcrCharacterButtons.size(); ++button)
        {
            SecureZeroMemory(&characterButtons[button], sizeof(characterButtons[button]));
            if (!m_dcrCharacterButtons[button]->GetText().empty())
            {
                wcscpy_s(characterButtons[button].text,
                         _countof(characterButtons[button].text),
                         m_dcrCharacterButtons[button]->GetText().c_str());
            }
        }
        Data_t* pData = new (pBuffer)
            Data_t(GetClass().c_str(),
                   m_dcrCharacterEdit.GetText(),
                   m_dcrServerEdit.GetText(),
                   characterButtons);
        GetPipelineManager().Callback(pData);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // Translate
} // Eq2Login
} // Broker

////////////////////////////////////////////////////////////////////////////////
