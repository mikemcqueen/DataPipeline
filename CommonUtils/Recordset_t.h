
#pragma once

class Recordset_t :
    public CRecordset
{

public:

    static const DWORD DefaultReadOnlyFlags = CRecordset::readOnly |
                                              CRecordset::executeDirect |
                                              CRecordset::noDirtyFieldCheck;

    typedef unsigned Field_t;

private:

    Field_t m_Fields;
    bool    m_bAllowDefaultConnect;

public:

    Recordset_t(
        CDatabase* pdb,
        bool       bAllowDefaultConnect)
    :
        CRecordset(pdb),
        m_bAllowDefaultConnect(bAllowDefaultConnect)
    {
        m_Fields = 0;
    }

    bool CheckField(Field_t Field) const { return (m_Fields & Field) == Field; }

    void ClearFields()                   { m_Fields = 0; }

    void AddField(Field_t Field)
    {
        if (!CheckField(Field))
        {
            m_Fields |= Field;
            ++m_nFields;
        }
    }

    void AddParam(Field_t Param)
    {
        if (!CheckField(Param))
        {
            m_Fields |= Param;
            ++m_nParams;
        }
    }

    bool AllowDefaultConnect() const { return m_bAllowDefaultConnect; }

private:

    // Explicitly disabled:
    Recordset_t();

};

