#ifndef _FILE_BLOB_CACHE_H_
#define _FILE_BLOB_CACHE_H_


#include "BlobCache.h"
#include <string>

namespace cache {

uint32_t GenerateCRC32(const uint8_t *data, size_t size);

class FileBlobCache : public BlobCache {
public:
    // FileBlobCache attempts to load the saved cache contents from disk into
    // BlobCache.
    FileBlobCache(size_t maxKeySize, size_t maxValueSize, size_t maxTotalSize,
            const std::string& filename);

    // writeToFile attempts to save the current contents of BlobCache to
    // disk.
    void writeToFile();

    // Return the total size of the cache
    size_t getSize();

private:
    // mFilename is the name of the file for storing cache contents.
    std::string mFilename;
};

} // namespace cache

#endif //_FILE_BLOB_CACHE_H_