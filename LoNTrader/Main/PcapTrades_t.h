/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PcapTrades_t.h
//
// Packet capture task (Acquire Handler).
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_PCAPTRADES_T_H
#define Include_PCAPTRADES_T_H

#include "PcapTask_t.h"
#include "Trade_t.h"

/////////////////////////////////////////////////////////////////////////////

// TODO: namespace Pcap {
//          namespace Interpret { struct Data_t {} }
//          namespace Acquire { class Handler_t {} }
//       }

class PcapTrades_t :
    public PcapTask_t
{

public:

    struct AcquireData_t :
        public DP::Message::Data_t
    {
        TradeId_t TradeId;
    };

public:

    PcapTrades_t();

/*
	virtual
    ~PcapTrades_t();
*/

    //
    // PcapTask virtual:
    //

    virtual
    void
    PostData(
        const unsigned char* pData);

private:

    TradeId_t
    IsRemovePacket(
        const unsigned char* pkt_data);

private:

    PcapTrades_t(const PcapTrades_t&);
    PcapTrades_t& operator=(const PcapTrades_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_PCAPTRADES_T_H

/////////////////////////////////////////////////////////////////////////////
