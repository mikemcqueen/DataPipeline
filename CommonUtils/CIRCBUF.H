//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// cheezoid circular buffer

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _CIRCBUF_H_
#define _CIRCBUF_H_

template<class T>
class circular_buffer : private std::vector<T>
{
    static const size_t DefaultSize = 16;

protected:
    int     m_iHead;
    int     m_iTail;
#ifdef CBUF_MAXSIZE
    int     m_iMaxSize;
#endif

public:
    circular_buffer()
    {
        init(DefaultSize);
    }

    circular_buffer(size_t Size)
    {
        init(Size);
    }

    void clear( void )
    {
        m_iHead = -1;
        m_iTail = -1;
    }

    T& push_back()
    {
        if (++m_iTail == size())
            m_iTail = 0;
        if (m_iTail == m_iHead)
        {
            _putws(L"push_back failed.");
            throw E_FAIL;
#if 0
            pElem = at(m_iHead);
            if (++m_iHead==size())
                m_iHead = 0;
#endif
        }
        else if (-1 == m_iHead)
             m_iHead = m_iTail;
        return at(m_iTail);

#ifdef CBUF_MAXSIZE
        int iLength = length();
        if( m_iMaxSize < iLength )
            m_iMaxSize = length();
#endif
    }

    T& pop_front( void )
    {
        if( -1 == m_iHead )
        {
            _putws(L"pop_front() on empty buffer");
            throw E_FAIL;
        }
        int iPos = m_iHead;
        if( m_iHead==m_iTail )
            clear();
        else if( size()<=++m_iHead )
            m_iHead = 0;
        return at( iPos );
    }

    bool empty( void )
    {
        return -1 == m_iHead;
    }

    int length( void )
    {
        int iLen;
        if( m_iHead==-1 )
            iLen = 0;
        else if( m_iHead<=m_iTail )
            iLen = m_iTail-m_iHead+1;
        else
            iLen = size()-m_iHead+m_iTail+1;
        return iLen;
    }

    T& operator[]( int idx )
    {
        if( idx<0 || length()<=idx )
            throw idx;
        idx += m_iHead;
        if( size()<=idx )
            idx -= size();
        return at( idx );
    }

    T& at_tail( int idx )
    {
        if( idx<0 || length()<=idx )
            throw idx;
        idx = m_iTail-idx;
        if( idx<0 )
            idx += size();
        return at( idx );
    }

private:
    void
    init(size_t Size)
    {
        resize( Size );
        clear();
#ifdef CBUF_MAXSIZE
        m_iMaxSize = 0;
#endif
    }


private:
    circular_buffer(const circular_buffer&);
    circular_buffer& operator=(const circular_buffer&);
};

#endif // _CIRCBUF_H_
