///////////////////////////////////////////////////////////////////////////////

template<class Host_t> // , typename Param_t = void>
class XmlFile_t
{

private:

    const Host_t*       m_pHost;
    const wchar_t*      m_pszFilename;
    CComPtr<IXmlWriter> m_pWriter;
    CComPtr<IXmlReader> m_pReader;
//    Param_t             m_Param;

public:

//    template<typename Param_t>
    XmlFile_t(
        const Host_t*  pHost,
        const wchar_t* pszFilename)
//            Param_t  Param)
        :
        m_pHost(pHost),
        m_pszFilename(pszFilename)
//        m_Param(Param)
    { }

    template<typename Param_t>
    bool
    Write(
        Param_t Param)
    {
        LogAlways(L"WriteXml_t::Write(%ls)", m_pszFilename);
        return WriteStart() &&
               m_pHost->WriteXml(m_pWriter, Param) &&
               WriteEnd();
    }

    bool
    Write()
    {
        LogAlways(L"WriteXml_t::Write(%ls)", m_pszFilename);
        return WriteStart() &&
               m_pHost->WriteXml(m_pWriter) &&
               WriteEnd();
    }

    template<typename Param_t>
    bool
    Read(
        Param_t Param)
    {
        LogAlways(L"ReadXmlFile(%ls)", m_pszFilename);
        CComPtr<IXmlReader> pReader;
        if (!util::CreateXmlReader(&pReader, m_pszFilename))
            return false;
        return m_pHost->ReadXml(pReader, Param);
    }

    bool
    Read()
    {
        LogAlways(L"ReadXmlFile(%ls)", m_pszFilename);
        CComPtr<IXmlReader> pReader;
        if (!util::CreateXmlReader(&pReader, m_pszFilename))
            return false;
        return const_cast<Host_t*>(m_pHost)->ReadXml(pReader);
    }

private:

    bool
    WriteStart()
//        const wchar_t* pszFilename)
    {
        if (!util::CreateXmlWriter(&m_pWriter, m_pszFilename))
            return false;
        HRESULT hr = m_pWriter->WriteStartDocument(XmlStandalone_Omit);
        if (FAILED(hr))
        {
            LogError(L"WriteStartDocument() failed (%08x)", hr);
            return false;
        }
        return true;
    }

    bool
    WriteEnd()
    {
        HRESULT hr;
        hr = m_pWriter->WriteEndDocument();
        if (FAILED(hr))
        {
            LogError(L"WriteEndDocument() failed (%08x)", hr);
            return false;
        }
        hr = m_pWriter->Flush();
        if (FAILED(hr))
        {
            LogError(L"Flush() failed (%08x)", hr);
            return false;
        }
        return true;
    }
};

namespace util
{

template<class T>
bool
WriteXmlFile(
    const T* pClass,
    const wchar_t* pszFilename)
{
    LogAlways(L"WriteXmlFile(%ls)", pszFilename);

    HRESULT hr;
    CComPtr<IXmlWriter> pWriter;
    if (!util::CreateXmlWriter(&pWriter, pszFilename))
        return false;
    hr = pWriter->WriteStartDocument(XmlStandalone_Omit);
    if (FAILED(hr))
    {
        LogError(L"WriteStartDocument() failed (%08x)", hr);
        return false;
    }

    if (!pClass->WriteXml(pWriter))
        return false;

    if (FAILED(hr = pWriter->WriteEndDocument()))
    {
        LogError(L"WriteEndDocument() failed (%08x)", hr);
        return false;
    }
    if (FAILED(hr = pWriter->Flush()))
    {
        LogError(L"Flush() failed (%08x)", hr);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
bool
ReadXmlFile(
          T*       pClass,
    const wchar_t* pszFilename)
{
    LogAlways(L"ReadXmlFile(%ls)", pszFilename);

    CComPtr<IXmlReader> pReader;
    if (!util::CreateXmlReader(&pReader, pszFilename))
        return false;
    return pClass->ReadXml(pReader);
}

///////////////////////////////////////////////////////////////////////////////

}; // namespace util

///////////////////////////////////////////////////////////////////////////////
