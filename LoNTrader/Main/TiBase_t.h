/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TiBase_t.h
//
// Text Interpreter base class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TIBASE_T_H
#define Include_TIBASE_T_H

#include "TextTable_t.h"
#include "AutoHand.h"
#include "Pool.h"
#include "CircBuf.h"
#include "Log.h"

/////////////////////////////////////////////////////////////////////////////

template<
    size_t LineCount,
    size_t CharsPerLine,
    size_t ColumnCount,
    size_t LineHeight,
    size_t CharHeight>
class TiBase_t
{
private:
    static const bool s_EXTRALOG = true;

public:

	typedef TextTable_t<LineCount, CharsPerLine, ColumnCount> Text_t;
	// LineHeight, CharHeight > Text_t;
//    typedef TextTable_t<LineCount, CharsPerLine, ColumnCount, LineHeight, CharHeight> Table_t;

protected:

    Text_t  m_Text;
    size_t  m_SameTextCount;

public:

    TiBase_t(
        const size_t* pColumnWidths,
        size_t        ColumnCount)
        :
        m_Text(pColumnWidths, ColumnCount),
        m_SameTextCount(0)
    {
    }

    virtual
    ~TiBase_t()
    { }

    //
    // Helpers.
    //

    bool
    CompareText(
        const Text_t& text,
              size_t&          FirstNewLine)
    {
#if 0
        if (s_EXTRALOG)
            text.Dump(L"Incoming text:", false);
#endif
        bool bNewText = FindFirstNewLine(text, FirstNewLine);
        if (bNewText)
        {
            ASSERT(text.GetEndRow() > FirstNewLine);
            if (s_EXTRALOG)
            {
                size_t Count = text.GetEndRow() - FirstNewLine;
                LogInfo(L"New lines (%d)", Count);
            }
            m_SameTextCount = 0;
        }
        else
        {
            ++m_SameTextCount;
            if (s_EXTRALOG)
                LogInfo(L"Same text (%d)", m_SameTextCount);
        }
        return bNewText;
    }

    size_t
    GetSameTextCount() const
    {
        return m_SameTextCount;
    }

    void
    SetText(
        const Text_t& Text)
    {
		// m_Text = Text; // commented out this line and
        m_Text = Text.GetData(); // changed to this line to compile
        if (s_EXTRALOG)
            m_Text.Dump(L"Saved text:");
    }

    const Text_t&
    GetText() const
    {
        return m_Text;
    }

    void
    ClearText()
    {
		// NOTE: Added GetData() here so it'll compile, but why is TextTable_t::Clear() private? 
        m_Text.Clear();
    }

    bool
    IsTextEmpty() const
    {
        return m_Text.IsEmpty();
    }

private:

	bool
    FindFirstNewLine(
        const Text_t& text,
              size_t& iFirst) const
    {
        iFirst = 0;

        // If the supplied text is empty, there are no new lines.
        if (text.IsEmpty())
        {
            if (s_EXTRALOG)
                LogInfo(L"Supplied text is empty");
            return false;
        }

        // If m_text is empty, the first new line is zero.
        if (m_Text.IsEmpty())
        {
            if (s_EXTRALOG)
                LogInfo(L"m_text is empty");
            return true;
        }

        ASSERT((0 < m_Text.GetEndRow()) && (0 < text.GetEndRow()));
        size_t MyLastLine  = m_Text.GetEndRow() - 1;
        if (1 > MyLastLine)
        {
            // TODO: So what? with the proper state machine set up, this should
            // be acceptable.
            if (s_EXTRALOG)
                LogInfo(L"Not enough lines match (MyLastLine)");
            return true;
        }
        size_t TheLastLine = text.GetEndRow() - 1;
        if (s_EXTRALOG)
        {
            LogInfo(L"TheLastLine=%d:", TheLastLine);
            text.DumpRow(TheLastLine);
            LogInfo(L"MyLastLine=%d:",  MyLastLine);
            m_Text.DumpRow(MyLastLine);
        }

        // Find a line in the supplied text that matches the last line in m_text.
        size_t Line = TheLastLine;
        while (0 <= Line)
        {
            if (!MatchLine(MyLastLine, text, Line))
            {
                if (s_EXTRALOG)
                    LogInfo(L"Last line not matched");
                return true;
            }

            if (0 == Line)  // || (0 == MyLastLine)) // cannot be, checked above
            {
                // TODO: So what? with the proper state machine set up, this should
                // be acceptable.
                if (s_EXTRALOG)
                    LogInfo(L"Not enough lines matched (Line)");
                return true;
            }

            if (s_EXTRALOG)
                LogInfo(L"Matched line %d", Line);

            // We have found a line in the supplied text which matches the the 
            // last line of m_text.
            // Compare all previous lines in the supplied text with the corresponding
            // previous lines in m_text.
            size_t Count = CompareLines(MyLastLine, text, Line);
            if (Count == Line)
            {
                // If the matched line is the last line in the supplied text,
                // then there can be no new supplied text.
                if (Line == TheLastLine)
                {
                    if (s_EXTRALOG)
                        LogInfo(L"No change");
                    // no change
                    return false;
                }
                iFirst = Line + 1;
                if (s_EXTRALOG)
                    LogInfo(L"Full match, First = %d", iFirst);
                return true;
            }

            if (s_EXTRALOG)
                LogInfo(L"Previous line didn't match (%d, %d, %d)",
                        Count,
                        MyLastLine - (Count - 1),
                        Line - (Count - 1));

            if (0 == Line--)
                break;
        }
        return true;
    }

    // TODO: take a direction (1,-1)
    bool
    MatchLine(
        size_t        MyLine,
        const Text_t& text,
        size_t&       TheLine) const
    {
        while (0 <= TheLine)
        {
            if (m_Text.CompareRow(MyLine, text, TheLine))
                return true;
            if (0 == TheLine--)
                break;
        }
        return false;
    }

    size_t
    CompareLines(
        size_t        MyLine,
        const Text_t& text,
        size_t        TheLine) const
    {
        size_t Count = 0;
        ASSERT((0 < MyLine) && (0 < TheLine));
        for(; (0 < MyLine) && (0 < TheLine); ++Count)
        {
            if (!m_Text.CompareRow(--MyLine, text, --TheLine))
            {
                if (s_EXTRALOG)
                {
                    LogInfo(L"CompareLine failed.", MyLine, TheLine);
                    m_Text.DumpRow(MyLine, L"   My:");
                    text.DumpRow(TheLine, L"  The:");
                }
                break;
            }
        }
        return Count;
    }

private:

    TiBase_t();
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TIBASE_T_H

/////////////////////////////////////////////////////////////////////////////
