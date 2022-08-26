
namespace DiffDb
{
    namespace CountType
    {
        enum Type : size_t
        {
            NetSales,
            GrossSales,
            Volume,

            Default      = NetSales
        };
    }
    typedef size_t CountType_t;

    class Base_t;
}

typedef DiffDb::Base_t DiffDb_t;

