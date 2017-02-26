
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Card_t.h"
#include "Log.h"

static const CardId_t DummyCardId     = 0xB00B1E5;
static const wchar_t  DummyCardName[] = L"DummyCard!";
static const size_t   DummyCardValue  = 0;

Card_t Card_t::DummyCard(DummyCardId, DummyCardName, DummyCardValue);

/////////////////////////////////////////////////////////////////////////////

Card_t::
Card_t(
    Token_t Token)
:
    m_id(0),
    m_Value(0)
{
Token;
    ASSERT(ClassToken == Token);
}

/////////////////////////////////////////////////////////////////////////////

Card_t::
Card_t(
    CardId_t       id,
    const wchar_t* pszName,
    size_t         Value)
: 
    m_id(id),
    m_strName(pszName),
    m_Value(Value)
{
}

/////////////////////////////////////////////////////////////////////////////

Card_t::
~Card_t()
{
}

/////////////////////////////////////////////////////////////////////////////

void
Card_t::
SetValue(
   size_t Value,
   bool   bQuiet)
{
    if (!bQuiet)
        LogInfo(L"Card_t::SetValue(): '%ls' (%d) (%d)", GetName(), m_Value, Value);
    m_Value = Value;
}

/////////////////////////////////////////////////////////////////////////////
