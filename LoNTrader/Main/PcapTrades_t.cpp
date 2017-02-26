/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PcapTrades_t.cpp
//
// Lon Trades Pcap acquire handler.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define HAVE_REMOTE
//#include <Pcap.h>
#include "PipelineManager.h"
#include "TradeManager_t.h"
#include "PcapTrades_t.h"
#include "Services.h"
#include "Log.h"
#include "Macros.h"
#include "Trade_t.h"

//#define FAST

/////////////////////////////////////////////////////////////////////////////

namespace Lon
{

#pragma pack(push)
#pragma pack(1)

struct ether_header
{
    u_char data[14];
};

struct ip_header
{
    u_char data[20];
};

struct tcp_header
{
    u_char data[20];
};

struct Packet_t
{
    struct Header_t
    {
        ether_header ether;
        ip_header    ip;
        tcp_header   tcp;
    } Header;

    u_char unknown1[2];
    u_char SizeHi;
    u_char SizeLo;

    unsigned int Size() const { return MAKEWORD(SizeLo, SizeHi); }

    struct Triplet_t
    {
        u_char a;
        u_char b;
        u_char c;

        bool
        Compare(
            const Triplet_t& t) const
        {
            return (t.a == a) && (t.b == b) && (t.c == c);
        }
    };

    Triplet_t Op1;
    Triplet_t two;
    Triplet_t three;

    u_char zero;

    struct TradeNumber_t
    {
        struct
        {
            u_char bytes[4];
        };
        long
        GetNumber() const
        {
            return (long)(bytes[3] << 12) | (bytes[2] << 4) | (bytes[1] & 0x0f);
        }
    } num;
};

#pragma pack(pop)

}; // namepsace Lon

/////////////////////////////////////////////////////////////////////////////

PcapTrades_t::
PcapTrades_t()
{
}

/////////////////////////////////////////////////////////////////////////////

TradeId_t
PcapTrades_t::
IsRemovePacket(
    const unsigned char* pkt_data)
{
    const Lon::Packet_t::Triplet_t RemoveOp = { 0x2f, 0x18, 0x01 };

    Lon::Packet_t& Packet = (Lon::Packet_t&)*pkt_data;
    if (1000 < Packet.Size())
    {
        LogWarning(L"PCAP: PacketSize(%d)", Packet.Size());
        return 0;
    }
    if (Packet.Op1.Compare(RemoveOp))
    {
        ASSERT(14 == Packet.Size());
        TradeId_t TradeId(Packet.num.GetNumber());
        LogInfo(L"PCAP: Remove (%d)", TradeId);
        /*
          "(%02X %02X %02X %02X)",
          Packet.num.bytes[0], Packet.num.bytes[1],
          Packet.num.bytes[2], Packet.num.bytes[3]);
        */
        return TradeId;
    }
    else
    {
    #ifdef EXTRALOG
        LogInfo(L"Packet: Bytes = %02X %02X %02X, Size (%d)",
                Packet.one.a, Packet.one.b, Packet.one.c, Packet.Size());
    #endif
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////


void
PcapTrades_t::
PostData(
    const unsigned char* pkt_data)
{
    TradeId_t TradeId = IsRemovePacket(pkt_data);
    if (0 == TradeId)
        return;
    AcquireData_t *pData = static_cast<AcquireData_t*>
        (GetPipelineManager().Alloc(sizeof AcquireData_t));
    if (NULL == pData)
    {
        LogError(L"PCAP: Alloc callback data failed.");
    }
    else
    {
        wcscpy_s(pData->Class, _countof(pData->Class), GetClass().c_str());
        pData->Stage    = DP::Stage::Interpret;
        pData->Type     = DP::Message::Type::Message;
        pData->Id       = Lon::Message::Id::RemoveTrade;
        pData->TradeId  = TradeId;

        HRESULT hr = GetPipelineManager().Callback(pData);
        if (FAILED(hr))
            LogError(L"PCAP: PM.Callback() failed.");
    }
}		

/////////////////////////////////////////////////////////////////////////////
