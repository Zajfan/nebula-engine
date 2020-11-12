//------------------------------------------------------------------------------
//  querybenchmark.cc
//  (C) 2020 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "stdneb.h"
#include "querybenchmark.h"
#include "memdb/table.h"
#include "memdb/typeregistry.h"
#include "memdb/database.h"

namespace Benchmarking
{
__ImplementClass(Benchmarking::QueryBenchmark, 'QMPB', Benchmarking::Benchmark);

using namespace Timing;
using namespace MemDb;

uint PsuedoRand()
{
    // XORshift128
    static uint x = 123456789;
    static uint y = 362436069;
    static uint z = 521288629;
    static uint w = 88675123;
    uint t;
    t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

//------------------------------------------------------------------------------
/**
*/
void
QueryBenchmark::Run(Timer& timer)
{   
    // Create a database
    Ptr<Database> db = Database::Create();
    
    // Setup a multitude of descriptors
    Util::Array<PropertyId> d;
    const SizeT numDescs = 1024;
    for (int i = 0; i < numDescs; i++)
    {
        Util::String name = "BenchType_" + Util::String::FromInt(i);
        d.Append(TypeRegistry::Register<int>(name, 0));
    }

    using DA = Util::FixedArray<PropertyId>;

    // Create a couple of tables
    for (int i = 0; i < Database::MAX_NUM_TABLES; i++)
    {
        TableCreateInfo info;
        info.name = "Table_" + Util::String::FromInt(i);

        const SizeT numColumns = (numDescs / 24);
        DA da = DA(numColumns);
        for (int a = 0; a < numColumns; a++)
        {
            IndexT descriptorIndex = PsuedoRand() % numDescs;
            da[a] = d[descriptorIndex];
        }

        info.columns = da;
        db->CreateTable(info);
    }

    /*Util::Array<TableMask> masks;
    const SizeT numQueries = 20000;
    for (int i = 0; i < numQueries; i++)
    {
        const SizeT numBitsSet = (numDescs / 8);
        DA da = DA(numBitsSet);
        for (int a = 0; a < numBitsSet; a++)
        {
            IndexT descriptorIndex = PsuedoRand() % numDescs;
            da[a] = d[descriptorIndex];
        }
        TableMask mask = TableMask(da);
        masks.Append(mask);
    }*/

    Util::Array<FilterSet> masks;
    const SizeT numQueries = 100000;
    for (int i = 0; i < numQueries; i++)
    {
        const SizeT numBitsSet = (numDescs / 8);
        DA da = DA(numBitsSet);
        for (int a = 0; a < numBitsSet; a++)
        {
            IndexT descriptorIndex = PsuedoRand() % numDescs;
            da[a] = d[descriptorIndex];
        }
        FilterSet mask;
        mask.inclusive = da.AsArray();
        masks.Append(mask);
    }

    timer.Start();
    for (int i = 0; i < numQueries; i++)
    {
        volatile Dataset data = db->Query(masks[i]);
    }
    timer.Stop();
}

} // namespace Benchmarking