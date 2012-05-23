#ifndef ALIGNMENTUTILS_BAMPOOL_H
#define ALIGNMENTUTILS_BAMPOOL_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "api/BamAux.h"
#include "api/BamWriter.h"

#include "Alignment.h"
#include "BamMultiReader.h"

#define DEFAULT_MAX_SIZE 10000000
#define DEFAULT_TMP_DIR "/tmp"

using std::string;
using std::vector;

template<class Compare>
class BamPoolReader
{
    struct ReadHead
    {
        BamReader* reader;
        Alignment alignment;

        bool operator< (const ReadHead& lhs, const ReadHead& rhs)
        {
            return comparator(lhs.alignment, rhs.alignment);
        }

        bool readNext(void)
        {
            return reader->GetNextAlignment(alignment);
        }
    }

    bool initialized = false;
    Compare comparator;
    vector<string> file_names;
    vector<ReadHead*> read_heads;

    void init (void);

    public:
        BamPoolReader (vector<string> fns) : file_names(fns);

        bool GetNextAlignment(Alignment& alignment);
};

template<class Compare>
class BamPool
{
    string TMP_DIR;
    int MAX_SIZE;

    typedef vector<Alignment> buffer_t;
    buffer_t buffer;

    vector<string> file_names;
    BamTools::RefVector references;

    Compare comparator;

    public:

        BamPool (BamTools::RefVector& refs,
                 int max_size = DEFAULT_MAX_SIZE,
                 string tmp_dir = DEFAULT_TMP_DIR) :
                 references(refs),
                 MAX_SIZE(max_size),
                 TMP_DIR(tmp_dir) {};
        ~BamPool (void);

        void add (BamTools::BamAlignment&);
        void flush (void);

        BamPoolReader getReader(void)
        {
            return BamPoolReader(file_names);
        }
};
#endif
