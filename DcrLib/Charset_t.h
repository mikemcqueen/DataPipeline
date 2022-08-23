/////////////////////////////////////////////////////////////////////////////
//
// Charset_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_CHARSET_T_H
#define Include_CHARSET_T_H

/////////////////////////////////////////////////////////////////////////////

struct CharData_t
{
    DWORD		dwFlags;
    POINT		ptOffset;
    int			iFirstKernPair;
    size_t      LeftPixelCount;
    ABC         ABCWidths;

    size_t      Width;
    int         cLeadingPixels;
    int         cTrailingPixels;
};

/////////////////////////////////////////////////////////////////////////////

typedef std::vector<CharData_t>		CharDataVector;
typedef std::vector<KERNINGPAIR>	KernPairVector;

/////////////////////////////////////////////////////////////////////////////

class CSurface;

class Charset_t
{

    static const COLORREF DefaultBkColor   = RGB(  0,   0,   0);
    static const COLORREF DefaultTextColor = RGB(255, 255, 255); 

private:

	mutable std::vector<POINT> m_vecOverlapPoints;
	std::vector<wchar_t>  m_vszCharset;
	CharDataVector        m_vCharData;
	KernPairVector        m_vKernPairs;
	std::vector<int>      m_viDx;
	CSurface*             m_spSurface;
	int                   m_iSpaceWidth;
    bool                  m_valid;
    size_t                m_charsetSize;

public:	

    explicit
	Charset_t(
        HFONT hFont,
        const wchar_t* pszCharset);

    explicit
	Charset_t(
        const LOGFONT& lf,
        const wchar_t* pszCharset );

	~Charset_t();

//    CSurface* GetSurface()  { return m_spSurface; }

	void SetSpaceWidth(int iWidth) { m_iSpaceWidth = iWidth; }
	int  GetSpaceWidth(void) const { return m_iSpaceWidth; }

	void SetCharFlags(wchar_t ch, DWORD dwFlags);
	void SetCharFlags(const wchar_t* pszChars, DWORD dwFlags);

	const CharData_t& GetCharData(unsigned Char) const;
//	CharData_t&       GetCharData(unsigned Char);

    void SetCharWidths(wchar_t ch, const ABC& abc);


	wchar_t  GetChar(unsigned Char) const;

	HRESULT
    Compare(
        size_t          cLeftPixels,
        const CSurface* pSurface,
        int&            x,
        int&            y,
        const RECT*     pRect,
        unsigned&       iChar,
        DWORD           dwFlags = 0) const;

	int
    GetKernAmount(
        unsigned FirstChar,
        unsigned SecondChar) const;

    static
    size_t
    FindNextChar(
        const CSurface *pSurface,
              int      &x,
              int      &y,
        const RECT *    pRect=0,
              DWORD     dwFlags = 0);

	static int
    SkipChar( const CSurface *pSurface, int& x, int& y, const RECT *pRect=0 );
	
    void 
    WriteBmp(
        const wchar_t* pszFile,
        const RECT*    pRect = NULL) const;

    bool                    IsValid() { return m_valid; }

private:

    unsigned GetCharIndex(wchar_t ch);

    void Init(
        HFONT          hFont,
        const wchar_t* pszCharset);

	void					InitFontInfo( HFONT hFont, SIZE& size );
    void					GetKernPairs(const HDC hDC);
	void					DrawTextToSurface( HFONT hFont );
	void					InitCharData( void );

	HRESULT
    CompareChar(
        const CSurface* pSurface,
              int&      x,
              int&      y,
        const RECT*     pRect,
              unsigned  iChar,
              DWORD     dwFlags,
              size_t&   MatchCount) const;

	bool					MatchOverlapPointY( const POINT& pt, int& y ) const;

};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_CHARSET_T_H

/////////////////////////////////////////////////////////////////////////////
