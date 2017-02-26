///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradePosterData.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TradePoster.h"
#include "PassiveFixedPricing_t.h"
#include "SimpleRangePricing_t.h"
#include "SimplePosting_t.h"
#include "LonCard_t.h"
#include "Services.h"
#include "Log.h"
#include "XmlUtil.h"
#include "XmlFile.h"
#include "PostedTradesTypes.h"
#include "TradePosterData.h"

namespace TradePoster
{

///////////////////////////////////////////////////////////////////////////////
//
// Data_t
//
///////////////////////////////////////////////////////////////////////////////

const wchar_t Data_t::XmlElementName[]     = L"TradePoster";

const Data_t::TypeString_t
Data_t::TypeStringTable[Data_t::TypeCount] =
{
    Invalid, L"Invalid",
    Offer,   L"Offer",
    Want,    L"Want"
};

///////////////////////////////////////////////////////////////////////////////
//
// Used by Manager_t in ReadXmlFile
//

Data_t::
Data_t() :
    m_Id(0),
    m_Type(Invalid),
    m_pPricingPolicy(NULL),
    m_pPostingPolicy(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////

const wchar_t* 
Data_t::
TypeToString(
    Type_e Type)
{
    return TypeStringTable[Type].String;
}

///////////////////////////////////////////////////////////////////////////////

Data_t::Type_e
Data_t::
StringToType(
    const wchar_t* String)
{
    for (size_t Type = 0; Type < TypeCount; ++Type)
    {
        if (0 == wcscmp(TypeStringTable[Type].String, String))
            return TypeStringTable[Type].Type;
    }
    LogError(L"TradedPoster::Data_t::StringToType(%ls) unknown type", String);
    throw std::invalid_argument("TradePoster::Data_t::StringToType()");
}

///////////////////////////////////////////////////////////////////////////////

PricingPolicy_i*
Data_t::
CreatePricingPolicy(
    const wchar_t* pName) const
{
    std::wstring PolicyName(pName);
    if (0 == PolicyName.compare(PassiveFixedPricing_t::PolicyName))
        return new PassiveFixedPricing_t;
    if (0 == PolicyName.compare(SimpleRangePricing_t::PolicyName))
        return new SimpleRangePricing_t;
    LogError(L"CreatePricingPolicy(%ls) Unknown policy", pName);
    throw std::invalid_argument("CreatePricingPolicy(Name)");
}

///////////////////////////////////////////////////////////////////////////////

PostingPolicy_i*
Data_t::
CreatePostingPolicy(
    const wchar_t* pName) const
{
    std::wstring PolicyName(pName);
    if (0 == PolicyName.compare(SimplePosting_t::PolicyName))
    {
        ASSERT(NULL != m_pPricingPolicy);
        return new SimplePosting_t(m_CardQ);
    }
    LogError(L"CreatePostingPolicy(%ls) Unknown policy", pName);
    throw std::invalid_argument("CreatePostingPolicy(Name)");
}

///////////////////////////////////////////////////////////////////////////////

bool
Data_t::
SetPricingPolicy(
    const wchar_t* pName)
{
    PricingPolicy_i* pPolicy = CreatePricingPolicy(pName);
    if (NULL == pPolicy)
        return false;
    SetPricingPolicy(pPolicy);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
Data_t::
SetPostingPolicy(
    const wchar_t* pName)
{
    PostingPolicy_i* pPolicy = CreatePostingPolicy(pName);
    if (NULL == pPolicy)
        return false;
    SetPostingPolicy(pPolicy);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
Data_t::
SetPricingPolicy(
    PricingPolicy_i* pNewPolicy)
{
    ASSERT(NULL != pNewPolicy);
    if (NULL != m_pPricingPolicy)
    {
        LogWarning(L"SetPricingPolicy() Old(%ls) New(%ls)",
                   m_pPricingPolicy->GetName(),
                   pNewPolicy->GetName());
        delete m_pPricingPolicy;
    }
    m_pPricingPolicy = pNewPolicy;
}

///////////////////////////////////////////////////////////////////////////////

void
Data_t::
SetPostingPolicy(
    PostingPolicy_i* pNewPolicy)
{
    ASSERT(NULL != pNewPolicy);
    if (NULL != m_pPostingPolicy)
    {
        LogWarning(L"SetPostingPolicy() Old(%ls) New(%ls)",
                   m_pPostingPolicy->GetName(),
                   pNewPolicy->GetName());
        delete m_pPostingPolicy;
    }
    m_pPostingPolicy = pNewPolicy;
}

///////////////////////////////////////////////////////////////////////////////

void
Data_t::
Show(
     size_t Value,
     bool   bDetail) const
{
    ASSERT(NULL != m_CardQ.pCard);
    if (NULL == m_CardQ.pCard)
        return;
    LogAlways(L"ID(%2d) %-4ls \"%-40ls\" %20ls %20ls",
              GetId(),
              TypeToString(GetType()),
              m_CardQ.pCard->GetName(),
              GetPricingPolicy().GetName(), 
              GetPostingPolicy().GetName());
    GetPricingPolicy().Show(bDetail);
    GetPostingPolicy().Show(Value, bDetail);
}

///////////////////////////////////////////////////////////////////////////////
//
// Get the value of this trade poster data as the sum of the supplied 
// CardValues of its cards.
//

size_t
Data_t::
GetValue(
    const CardValueSet_t& CardValues) const
{
    size_t TotalValue = 0;
    // foreach (Card in m_CardCollection_t)
    {
        CardValue_t Cv(m_CardQ.pCard);
        CardValueSet_t::const_iterator it = CardValues.find(Cv);
        if (CardValues.end() == it)
        {
            LogError(L"GetValue() No value for (%ls)", Cv.pCard->GetName());
            return 0;
        }
        const size_t CardValue = (Offer == GetType()) ? it->SellAt : it->BuyAt;
        TotalValue += CardValue;
    }
    return TotalValue;
}

///////////////////////////////////////////////////////////////////////////////
//
// Add a CardValue_t for each card in this trade poster data to the
// supplied CardValueSet_t.
// NOTE: Currently only support 1 card in per trade poster data, but that
// may change.
//

size_t
Data_t::
AddCardValues(
    CardValueSet_t& Cards) const
{
    // TODO: This would be the place for a loop or for_each on a 
    // CardCollection_t member, when we get to that point.
    return Cards.insert(CardValue_t(m_CardQ.pCard)).second ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////

bool
Data_t::
ReadXmlFile(
    const wchar_t* pszFilename)
{
    XmlFile_t<Data_t> XmlFile(this, pszFilename);
    return XmlFile.Read();
}

///////////////////////////////////////////////////////////////////////////////

bool
Data_t::
WriteXmlDirectory(
    const wchar_t* pszDir) const
{
    wchar_t szFile[MAX_PATH] = { 0 };
    if (GetPricingPolicy().GetLowPrice() == GetPricingPolicy().GetHighPrice())
    {
        _snwprintf_s(szFile, _TRUNCATE, L"%ls\\%ls_%ls_%d.xml",
                     pszDir,
                     (Offer == GetType()) ? L"Sell" : L"Buy",
                     m_CardQ.pCard->GetName(),
                     GetPricingPolicy().GetLowPrice());
    }
    else
    {        
        _snwprintf_s(szFile, _TRUNCATE, L"%ls\\%ls_%ls_%d-%d.xml",
                     pszDir,
                     (Offer == GetType()) ? L"Sell" : L"Buy",
                     m_CardQ.pCard->GetName(),
                     GetPricingPolicy().GetLowPrice(),
                     GetPricingPolicy().GetHighPrice());
    }
    LogAlways(L"  Writing: %d - %ls", GetId(), szFile);
    return WriteXmlFile(szFile);
}

///////////////////////////////////////////////////////////////////////////////

bool
Data_t::
WriteXmlFile(
    const wchar_t* pszFilename) const
{
    XmlFile_t<Data_t> XmlFile(this, pszFilename);
    return XmlFile.Write();
}

///////////////////////////////////////////////////////////////////////////////
//
// <TradePoster Type="Offer" Name="SellForMore">
//   <CardQuantity>
//   </CardQuantity>
//   <PricingPolicy Name="PricingPolicyName"/>
//   <PricingPolicyName>
//   </PricingPolicyName>
//   <PostingPolicy Name="PostingPolicyName"/>
//   <PostingPolicyName>
//   </PostingPolicyName>
// </TradePoster>
//

bool
Data_t::
ReadXml(
          IXmlReader* pReader,
    const wchar_t*    ElementName)
{
    using namespace XmlUtil;

    // <TradePoster>
    ReadElement_t TradePosterElement(pReader, ElementName);
    std::wstring Type;
    TradePosterElement.ReadAttribute(XmlAttributeType, Type);
    m_Type = StringToType(Type.c_str());

    //   <CardQuantity>
    if (!m_CardQ.ReadXml(pReader))
        return false;

    //   <PricingPolicy>
    std::wstring PricingPolicyName;
    ReadElement_t PricingPolicyElement(pReader, PricingPolicy_i::XmlElementName, false);
    PricingPolicyElement.ReadAttribute(XmlAttributeName, PricingPolicyName);
    if (!SetPricingPolicy(PricingPolicyName.c_str()))
        return false;
    if (!GetPricingPolicy().ReadXml(pReader, PricingPolicyName.c_str()))
        return false;

    //   <PostingPolicy>
    std::wstring PostingPolicyName;
    ReadElement_t PostingPolicyElement(pReader, PostingPolicy_i::XmlElementName, false);
    PostingPolicyElement.ReadAttribute(XmlAttributeName, PostingPolicyName);
    if (!SetPostingPolicy(PostingPolicyName.c_str()))
        return false;
    if (!GetPostingPolicy().ReadXml(pReader, PostingPolicyName.c_str()))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// <TradePoster Type="Offer" Name="SellForMore">
//   <CardQuantity>
//   </CardQuantity>
//   <PricingPolicy Name="PricingPolicyName"/>
//   <PricingPolicyName>
//   </PricingPolicyName>
//   <PostingPolicy Name="PostingPolicyName"/>
//   <PostingPolicyName>
//   </PostingPolicyName>
// </TradePoster>
//

bool
Data_t::
WriteXml(
          IXmlWriter* pWriter,
    const wchar_t*    ElementName) const
{
    using namespace XmlUtil;
    // <TradePoster>
    WriteElement_t TradePosterElement(pWriter, ElementName);
    TradePosterElement.WriteAttribute(XmlAttributeType, TypeToString(GetType()));
    if (!m_strName.empty())
    {
        TradePosterElement.WriteAttribute(XmlAttributeName, GetName());
    }
    //   <CardQuantity>
    m_CardQ.WriteXml(pWriter);

    //   <PricingPolicy>
    {
        WriteElement_t PricingPolicyElement(pWriter, PricingPolicy_i::XmlElementName);
        PricingPolicyElement.WriteAttribute(XmlAttributeName, GetPricingPolicy().GetName());
    }
    if (!GetPricingPolicy().WriteXml(pWriter, GetPricingPolicy().GetName()))
        return false;

    //   <PostingPolicy>
    {
        WriteElement_t PostingPolicyElement(pWriter, PostingPolicy_i::XmlElementName);
        PostingPolicyElement.WriteAttribute(XmlAttributeName, GetPostingPolicy().GetName());
    }
    if (!GetPostingPolicy().WriteXml(pWriter, GetPostingPolicy().GetName()))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

} // TradePoster
