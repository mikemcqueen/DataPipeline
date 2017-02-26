///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradePosterData.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_TRADEPOSTERDATA_H
#define Include_TRADEPOSTERDATA_H

#include "DpTransaction.h"
#include "TradePosterTypes.h"
#include "LonMessageTypes.h"
#include "CardCollection_t.h"
#include "Trade_t.h"

namespace TradePoster
{

    class Data_t
    {
        friend class Manager_t;

    public:

        static const wchar_t XmlElementName[];

        enum Type_e
        {
            Invalid,
            Offer,
            Want,
            TypeCount
        };

    private:

        struct TypeString_t
        {
            Type_e   Type;
            wchar_t* String;
        };
        static const TypeString_t TypeStringTable[];

    private:

        Flag_t           m_Flags;

        Id_t             m_Id;
        std::wstring     m_strName;
        CardQuantity_t   m_CardQ; // TODO: CardCollection_t
        Type_e           m_Type;

        PricingPolicy_i* m_pPricingPolicy;
        PostingPolicy_i* m_pPostingPolicy;

        // TimingPolicy_i* m_pTimingPolicy; // e.g. how often to post trades

    public:

        Data_t(
                  Id_t             InitId,
            const CardQuantity_t&  CardQ,
                  Type_e           InitType,
                  PricingPolicy_i* InitPricingPolicy,
                  PostingPolicy_i* InitPostingPolicy)
            :
            m_Flags(0),
            m_Id(InitId),
            m_CardQ(CardQ),
            m_Type(InitType),
            m_pPricingPolicy(InitPricingPolicy),
            m_pPostingPolicy(InitPostingPolicy)
        { }

        Id_t             GetId() const            { return m_Id; }
        const wchar_t*   GetName() const          { return m_strName.c_str(); }
        Type_e           GetType() const          { return m_Type; }

        PricingPolicy_i& GetPricingPolicy() const { return *m_pPricingPolicy; }
        PostingPolicy_i& GetPostingPolicy() const { return *m_pPostingPolicy; }

        void
        Show(
            size_t Value,
            bool   bDetail = false) const;

        bool
        WriteXmlDirectory(
            const wchar_t* pszDir) const;

        bool
        WriteXmlFile(
            const wchar_t* pszFilename) const;

        bool
        ReadXmlFile(
            const wchar_t* pszFilename);


        bool
        ReadXml(
                  IXmlReader* pReader,
            const wchar_t*    ElementName = XmlElementName);

        bool
        WriteXml(
                  IXmlWriter* pWriter,
            const wchar_t*    ElementName = XmlElementName) const;

        // Get the value of a trade as the sum of the supplied CardValues
        // of its cards.

        size_t
        GetValue(
            const CardValueSet_t& CardValues) const;

        // For each card in this trade poster data, add a CardValue_t to the
        // supplied CardValueSet_t.
        // NOTE: Currently we only support 1 card per trade poster data , but 
        //       that may change.

        size_t
        AddCardValues(
            CardValueSet_t& CardValues) const;

    private:

        PricingPolicy_i*
        CreatePricingPolicy(
            const wchar_t* pName) const;

        PostingPolicy_i*
        CreatePostingPolicy(
            const wchar_t* pName) const;

        bool
        SetPricingPolicy(
            const wchar_t* pName);

        bool
        SetPostingPolicy(
            const wchar_t* pName);

        void
        SetPricingPolicy(
            PricingPolicy_i* pNewPolicy);

        void
        SetPostingPolicy(
            PostingPolicy_i* pNewPolicy);

    private:

        static
        const wchar_t* 
        TypeToString(
            Type_e Type);

        static
        Type_e
        StringToType(
            const wchar_t* String);

    private:

        // Used by Manager_t
        Data_t();

    };

    ///////////////////////////////////////////////////////////////////////////////
    //
    // TradePoster::EventPost_t
    //
    ///////////////////////////////////////////////////////////////////////////////
   
    class EventPost_t
    {

    public:

        struct Data_t :
            public DP::Transaction::Data_t
        {
            const Trade_t* pTrade;        // [in]  copy of trade from TradePoster::Manager_t map.
            Id_t           TradePosterId; // [in]  Index in TradePoster::Manager_t::m_Map
            size_t         Value;         // [in]  Index in TradeMaker_t::m_Map
            bool           bTestPost;     // [in]  don't accept post, cancel out
            bool           bExitBuilder;  // [in]  exit trade builder
            TradeId_t      TradeId;       // [out] actual tradeid discovered in TiPostedTrades

            explicit
            Data_t(
                const Trade_t& InitTrade,
                      Id_t     InitTradePosterId,
                      size_t   InitValue,
                      bool     InitTestPost,
                      bool     InitExitBuilder)
            :
                DP::Transaction::Data_t(
                    Lon::Transaction::Id::PostTrade,
                    sizeof(Data_t)),
                pTrade(new Trade_t(InitTrade)),
                TradePosterId(InitTradePosterId),
                Value(InitValue),
                bTestPost(InitTestPost),
                bExitBuilder(InitExitBuilder),
                TradeId(0)
            { }

            ~Data_t()
            {
                delete pTrade;
            }

        private:

            Data_t();

        } m_Data;

        // TODO: change these bools to flags
        EventPost_t(
            const Trade_t& Trade,
                  Id_t     TradePosterId,
                  size_t   Value,
                  bool     bTestPost,
                  bool     bExitBuilder)
        :
            m_Data(
                Trade,
                TradePosterId,
                Value,
                bTestPost,
                bExitBuilder)
        { }

    private:

        EventPost_t();

    };

} // TradePoster

///////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEPOSTERDATA.H

///////////////////////////////////////////////////////////////////////////////
