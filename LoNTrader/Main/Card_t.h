/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Card_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_CARD_T_H
#define Include_CARD_T_H

typedef unsigned long CardId_t;

/////////////////////////////////////////////////////////////////////////////

class Card_t
{

friend class LonCardSet_t;

public:

    static Card_t DummyCard;

    typedef unsigned long Token_t;
    static const Token_t ClassToken = 0xB00B1E5;

protected:

    CardId_t     m_id;
    std::wstring m_strName;
    size_t       m_Value;

public:

    Card_t(
        Token_t Token);

    explicit
    Card_t(
        CardId_t       id,
        const wchar_t* pszName,
        size_t         Value);

    virtual
    ~Card_t();

    size_t         GetValue() const       { return m_Value; }

    const wchar_t* GetName() const        { return m_strName.c_str(); } 
    CardId_t       GetId() const          { return m_id; }

private:

    void
    SetName(const wchar_t* pName)
    {
        ASSERT(NULL != pName);
        m_strName.assign(pName);
    }

    void
    SetValue(
        size_t Value,
        bool   bQuiet = false);

    Card_t();
};

typedef std::vector<Card_t*> CardPtrVector_t;

#endif // Include_CARD_T_H

/////////////////////////////////////////////////////////////////////////////
