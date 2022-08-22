///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCardSet_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonCardSet_t.h"
#include "DbCards_t.h"
#include "Log.h"
//#include "TradeManager_t.h"
//#include "Timer_t.h"
//#include "Services.h"
#include "LonTrader_t.h"

//#include "boost/bind.hpp"

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
ShowNewCards(bool bDetail) const
{
   CLock lock(m_csNewCards);
   ShowCards(m_NewCards, false, bDetail, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
AddNewCard(
    const wchar_t* pszName) const
{
    ASSERT(NULL != pszName && L'\0' != pszName[0]);
    AddNewCard(LonCard_t(0, pszName, 0));
}

///////////////////////////////////////////////////////////////////////////////

bool
LonCardSet_t::
AddNewCard(
    const LonCard_t& Card) const
{
    CLock lock(m_csNewCards);
    std::pair<LonCardMap_t::iterator, bool> Insert;
    Insert = m_NewCards.insert(/*m_NewCards.begin(), */LonCard_t::Pair_t(Card.GetName(), Card));
    return Insert.second;
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
FoilifyNewCards()
{
    LonCardMap_t::iterator it = m_NewCards.begin();
    for (; m_NewCards.end() != it; ++it)
    {
        const LonCard_t& Card = it->second;
        switch(Card.GetNumber().type)
        {
        case L'C':
        case L'U':
        case L'R':
//        case L'P':
        case L'F':
            break;
        default:
            continue;
        }

        // State - We have a card in sets "CURF"

        LonCard_t::Number_t Number = Card.GetNumber();
        // See if its 'foil' opposite exists in the real card set.
        Number.foil = 1 - Number.foil;
        if (NULL != Lookup(Number))
            continue;

        // State - 'foil' opposite not found in real card set.
        // Create the 'foil' opposite card name.
        static const wchar_t szFoil[] = L" (foil)";
        std::wstring strNewName(Card.GetName());
        if (Number.foil)
            strNewName.append(szFoil);
        else
        {
            size_t Pos = strNewName.find(szFoil);
            if (std::wstring::npos != Pos)
                strNewName.erase(Pos);
            else
            {
                // somehow a new card got "foil" tagged but
                // doesn't contain the text " (foil)"...
                ASSERT(0);
            }
        }

        // Add card with 'foil' opposite name to the new card set.
        LonCard_t NewCard(0, strNewName.c_str(), 0);
        LogInfo(L"Foilify: Attempting to add '%ls'", NewCard.GetName());
        std::pair<LonCardMap_t::iterator, bool> Insert;
        Insert = m_NewCards.insert(LonCard_t::Pair_t(NewCard.GetName(), NewCard));
        it = Insert.first;
        LogInfo(L"Foilify: %ls", Insert.second ? L"Added." : L"Add failed.");

        // If new card isn't foil, the next card is, so skip it
        if (!NewCard.IsFoilCard())
            ++it;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
TruncateNewLootCards()
{
    LonCardMap_t::iterator it = m_NewCards.begin();
    for (; m_NewCards.end() != it; ++it)
    {
        LonCard_t& Card = it->second;
        if (CardType::Loot != Card.GetNumber().type)
            continue;

        std::wstring strName(Card.GetName());

        size_t CommaPos = strName.find(L", EverQ");
        if (strName.npos == CommaPos)
            continue;
        strName.erase(CommaPos);
        
        LogAlways(L"Truncating: '%ls' to '%ls'",
                  Card.GetName(),
                  strName.c_str());

        // bad?
        Card.SetName(strName.c_str());
/*
        LonCard_t NewCard(Card.GetId(), strName.c_str());
        std::pair<LonCardMap_t::iterator, bool> InsertResult;
        InsertResult = m_NewCards.insert(LonCard_t::Pair_t(NewCard.GetName(), NewCard));
        ASSERT(InsertResult.second);

        it = m_NewCards.erase(it);
*/
    }
}

///////////////////////////////////////////////////////////////////////////////

void
LonCardSet_t::
WriteNewCards()
{
    CLock lock(m_csNewCards);
    FoilifyNewCards();
    TruncateNewLootCards();
    CDatabase db;
    try
    {
        WriteNewCards(db);
    }
    catch(CDBException *e)
    {
        LogError(L"WriteNewCards() exception: %ls", e->m_strError);
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t
LonCardSet_t::
WriteNewCards(
    CDatabase& db)
{
    size_t Added = 0;

    BOOL b;
    b = db.Open(NULL, FALSE, FALSE, LonTrader_t::GetDbConnectString(), FALSE);
    ASSERT(!!b);

    CLock lock(m_csNewCards);
    LonCardMap_t::iterator it = m_NewCards.begin();
    while (m_NewCards.end() != it)
    {
        DbCards_t rs(&db);
        b = rs.Open(CRecordset::dynaset, NULL, CRecordset::appendOnly);
        ASSERT(!!b);
        LonCard_t& card = it->second;
        if (rs.CanAppend())
        {
            rs.AddNew();
            rs.m_cardname = card.GetName();
            rs.m_value    = 0;
            if (rs.Update())
            {
                rs.Requery();
                rs.SetAbsolutePosition(-1);
                rs.Close();
                LogAlways(L"Added '%ls' to DB (%d)", card.GetName(), rs.m_cardid);

                // Add card to main deck.
                LonCard_t NewCard(rs.m_cardid, card.GetName());
                AddCard(NewCard);
                // Erase card from newcard list.
                it = m_NewCards.erase(it);
                ++Added;
            }
            else
            {
                LogError(L"Can't update Db::Cards_t");
                ASSERT(false);
            }
        }
        else
        {
            LogError(L"Can't append to Db::Cards_t");
            ASSERT(false);
            ++it;
        }
        rs.Close();
    }
    db.Close();
    LogAlways(L"AddNewCards: %d added, %d remain", Added, m_NewCards.size());
    return Added;
}

///////////////////////////////////////////////////////////////////////////////
