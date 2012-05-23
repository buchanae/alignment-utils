#ifndef ALIGNMENTUTILS_ALIGNMENT_H
#define ALIGNMENTUTILS_ALIGNMENT_H

#include <string>

#include "api/BamAlignment.h"

#include "Feature.h"

using BamTools::BamAlignment;
using BamTools::CigarOp;

class Alignment : public BamAlignment
{
    public:
        std::string RefName;

        Alignment(void);
        Alignment(BamAlignment& other);
        bool getJunction(GFF::Feature&);
        int position(void);
        void position(int);

    private:
        // Prevent public access to Position, because it's zero-based,
        // which can cause confusion and difficult debugging.
        // Use position() instead.
        BamAlignment::Position;
};

string toString(vector<CigarOp>& cd)
{
    stringstream ss;
    string out = "";
    vector<CigarOp>::iterator iter = cd.begin();
    for( ; iter != cd.end() ; ++iter ){
        ss << iter->Length << iter->Type;
    }
    ss >> out;
    return out;
}

int sumCigar(vector<CigarOp>& cigar)
{
    int length = 0;
    for (unsigned int i = 0; i < cigar.size(); i++)
    {
        length += cigar.at(i).Length;
    }
    return length;
}

// Calculate paired-end gapped alignments using the CigarOp data
// NOT the length of the query sequence
int pairedGapLength(Alignment& a, Alignment& b)
{
    return b.position() - a.position() + sumCigar(a) - 2;
}
#endif
