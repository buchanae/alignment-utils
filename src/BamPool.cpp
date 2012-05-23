#include <algorithm>
#include <stdio.h>
#include <vector>

#include "api/BamAux.h"
#include "api/BamWriter.h"

#include "Alignment.h"
#include "BamMultiReader.h"

#include "BamPool.h"

using std::vector;
using std::string;

using BamTools::BamWriter;
using BamTools::RefVector;

template<class Compare>
void BamPoolReader<Compare>::init (void)
{
    // open files and initilize read heads
    for (vector<string>::iterator file_name = file_names.begin();
         file_name != file_names.end(); ++file_name)
     {
          BamReader* reader = new BamReader;
          if (!reader->Open(*file_name))
          {
              // TODO how to error here?
          }
          else
          {
              ReadHead* head = new ReadHead;
              head->reader = reader;
              if (head->readNext())
              {
                  read_heads.push_back(head);
              }
          }
     }

     // initialize heap of read heads
     make_heap(read_heads.begin(), read_heads.end());

     initialized = true;
}

template<class Compare>
bool BamPoolReader::GetNextAlignment(Alignment& alignment)
{
    if (!initialized) init();

    if (read_heads.empty()) return false;

    // next alignment is the one on the top of the heap
    ReadHead* max = read_heads.front();
    pop_heap(read_heads.begin(), read_heads.end());
    read_heads.pop_back();
    alignment = max->alignment;

    // read the next alignment from the read head we just popped off the heap
    if (max->readNext())
    {
        read_heads.push_back(max);
        push_heap(read_heads.begin(), read_heads.end());
    }
    else
    {
        max->reader->Close();
    }

    return true;
}

template<class Compare>
BamPool<Compare>::~BamPool ()
{
    // remove temporary files
    for (vector<string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        remove(it->c_str());
    }
}

template<class Compare>
void BamPool<Compare>::flush (void)
{
    if (buffer.empty()) return;

    std::sort(buffer.begin(), buffer.end(), comparator);

    // open temp. file
    string path = TMP_DIR + "/BamPool-XXXXXX";
    vector<char> dst_path(path.begin(), path.end());
    dst_path.push_back('\0');

    int fd = mkstemp(&dst_path[0]);
    if (fd != -1)
    {
        path.assign(dst_path.begin(), dst_path.end() - 1);
        close(fd);
    }
    // TODO throw error if file could not be created

    // open a BamWriter
    BamWriter writer;
    string header = "@HD\tVN:1.0\tSO:" + comparator.name;

    if (!writer.Open(path, header, references))
    {
        std::cerr << writer.GetErrorString() << std::endl;
        // TODO should really throw error here
        // TODO bad to assume we can use std::cerr
    }
    filenames.push_back(path);

    // dump buffer to file
    for (buffer_t::iterator it = buffer.begin(); it != buffer.end(); ++it)
    {
        writer.SaveAlignment(*it);
    }

    // TODO does closing the writer allow it to be removed?
    //      need to save reference to file handle?
    writer.Close();
    buffer.clear();
}

template<class Compare>
void BamPool<Compare>::add (BamAlignment& alignment)
{
    // flush buffer if it's full
    if (buffer.size() >= MAX_SIZE) flush();

    buffer.push_back(alignment);
}
