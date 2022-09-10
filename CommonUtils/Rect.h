////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Rect.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

class Rect_t :
    public RECT
{

public:

    Rect_t()
    {
        SetEmpty();
    }

    Rect_t(
        const RECT& rc)
    {
        CopyRect(this, &rc);
    }

    explicit
    Rect_t(
        POINT Point,
        const SIZE& Size)
    {
        SetRect(Point, Size);
    }

    Rect_t(int left, int top, int right, int bottom)
    {
        ::SetRect(this, left, top, right, bottom);
    }

/*
    const Rect_t&
    operator=(
        const Rect_t& rhs)
    {
        if (&rhs != this)
        {
            CopyRect(this, &rhs);
        }
        return *this;
    }
*/
    int Width() const     { return right - left; }
    int Height() const    { return bottom - top; }

    void SetEmpty()       { ::SetRectEmpty(this); }
    bool IsEmpty()        { return FALSE != ::IsRectEmpty(this); }

    POINT Center() const  { POINT pt = { left + Width() / 2, top + Height() / 2 }; return pt; }

    void
    SetRect(
        POINT Point,
        const SIZE& Size)
    {
        ::SetRect(this, Point.x, Point.y, Point.x + Size.cx, Point.y + Size.cy);
    }
};

////////////////////////////////////////////////////////////////////////////////

class RelativeRect_t : 
    public Rect_t
{

public:

    enum Flag_t : unsigned
    {
        // TODO: bit flags, include TopLeft = Top | Left, etc
        //       then we only need one "relativetype" in struct
        None     = 0,
        Left     = 0x0001,
        Right    = 0x0002,
        HCenter  = 0x0004,
        HorzMask = 0x00FF,
        Top      = 0x0100,
        Bottom   = 0x0200,
        VCenter  = 0x0400,
        VertMask = 0xFF00,

        LeftTop      = Left | Top,
        RightTop     = Right | Top,
        LeftBottom   = Left | Bottom,
        RightBottom  = Right | Bottom,
        CenterBottom = HCenter | Bottom,
    };

    struct Data_t
    {
        Flag_t Flags;
        int    x;
        int    y;
        int    Width;
        int    Height;
//        DWORD  Flags;
    };

private:

    Data_t m_Data;

public:

    explicit
    RelativeRect_t(
        const Data_t& Data)
    :
        m_Data(Data)
    { }

    // haxxor - just to support nullptr/EmptyRect
    RelativeRect_t(
        const Data_t* pData)
    {
        if (nullptr == pData)
        {
            SecureZeroMemory(&m_Data, sizeof(Data_t));
        }
        else
        {
            m_Data = *pData;
        }
    }

    const Rect_t&
    GetRelativeRect(
        POINT point)
    {
        Rect_t rect(point.x, point.y, point.x + 1, point.y + 1);
        return GetRelativeRect(rect);
    }

    const Rect_t&
    GetRelativeRect(
        const Rect_t& Rect)
    {
        switch (m_Data.Flags & HorzMask)
        {
        case None:     left = m_Data.x;                  break;
        case Left:     left = Rect.left + m_Data.x;      break;
        case Right:    left = Rect.right + m_Data.x;     break;
        case HCenter:  left = Rect.left + (Rect.Width() - m_Data.Width) / 2;  break;
        default: throw std::invalid_argument("RelativeRect Horz type");
        }
        switch (m_Data.Flags & VertMask)
        {
        case None:     top = m_Data.y;                   break;
        case Bottom:   top = Rect.bottom + m_Data.y;     break;
        case Top:      top = Rect.top + m_Data.y;        break;
        case VCenter:   top = Rect.top + (Rect.Height() - m_Data.Height) / 2; break;
        default: throw std::invalid_argument("RelativeRect Vert type");
        }
        right = left + m_Data.Width;
        bottom = top + m_Data.Height;
        return *this;
    }

    bool IsEmpty()        { return (0 == m_Data.Width) && (0 == m_Data.Height); }

private:

    // Explicitly disabled:
    RelativeRect_t();

    // No reason to not support these, but until needed, they are disabled:
    RelativeRect_t(const RelativeRect_t&);
    const RelativeRect_t& operator=(const RelativeRect_t&);

};

////////////////////////////////////////////////////////////////////////////////
