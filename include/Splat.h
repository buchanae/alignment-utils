#ifndef ALIGNMENTUTILS_SPLAT_H
#define ALIGNMENTUTILS_SPLAT_H

#include <string>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/join.hpp>

#include "Alignment.h"

using std::string;
using std::stringstream;
using std::vector;

struct SplatPosition
{
    int a_start;
    int a_end;
    int b_start;
    int b_end;

    bool operator== ( const SplatPosition& other ) const {
        return a_start == other.a_start && a_end == other.a_end 
               && b_start == other.b_start && b_end == other.b_end;
    }
};

struct Splat
{
    string ref;
    string flanks;
    SplatPosition pos;
    string seq;
    vector<string> read_IDs;

    string str(void)
    {
        stringstream buffer;

        buffer << ref << "\t";
        buffer << flanks << "\t";
        buffer << pos.a_end - pos.a_start + 1 << "\t";
        buffer << pos.b_end - pos.b_start + 1 << "\t";
        buffer << pos.b_start - pos.a_end - 1 << "\t";
        buffer << pos.a_start << "\t" << pos.a_end << "\t";
        buffer << pos.b_start << "\t" << pos.b_end << "\t";
        buffer << seq << "\t";
        buffer << read_IDs.size() << "\t";
        buffer << boost::algorithm::join(read_IDs, ",");

        return buffer.str();
    }

    bool shouldMerge (splat_t& other)
    {
        return ref == other.ref && pos == other.pos;
    }

    void merge (splat_t& other)
    {
        read_IDs.insert(read_IDs.end(), other.read_IDs.begin(), other.read_IDs.end());
    }
};

bool isSplat(Alignment& al)
{
    return al.HasTag("XD");
}

Splat toSplat (Alignment& al)
{
    Splat splat;

    splat.ref = al.RefName;
    al.GetTag("XD", splat.flanks);

    splat.pos.a_start = al.position();
    splat.pos.a_end = splat.pos.a_start + al.CigarData.at(0).Length - 1;
    splat.pos.b_start = splat.pos.a_end + al.CigarData.at(1).Length + 1;
    splat.pos.b_end = splat.pos.b_start + al.CigarData.at(2).Length - 1;

    splat.seq = al.QueryBases;

    string strand = al.IsReverseStrand() ? "-" : "+";

    stringstream read_ID (stringstream::in | stringstream::out);
    read_ID << strand;
    read_ID << al.Name;
    splat.read_IDs.push_back(read_ID.str());

    return splat;
}

#endif
