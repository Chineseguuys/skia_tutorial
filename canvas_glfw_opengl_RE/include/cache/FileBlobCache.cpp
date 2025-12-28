#include "FileBlobCache.h"
#include <fcntl.h>
#include <error.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <zlib.h>

#include <spdlog/spdlog.h>

// Cache file header
static const char* cacheFileMagic = "EGL$";
static const size_t cacheFileHeaderSize = 8;

namespace cache {

uint32_t GenerateCRC32(const uint8_t *data, size_t size)
{
    const unsigned long initialValue = crc32_z(0u, nullptr, 0u);
    return static_cast<uint32_t>(crc32_z(initialValue, data, size));
}

FileBlobCache::FileBlobCache(size_t maxKeySize, size_t maxValueSize, size_t maxTotalSize,
    const std::string& filename)
    : BlobCache(maxKeySize, maxValueSize, maxTotalSize)
    , mFilename(filename) {

    if (mFilename.length() > 0) {
        size_t headerSize = cacheFileHeaderSize;

        int fd = open(mFilename.c_str(), O_RDONLY, 0);
        if (fd == -1) {
            if (errno != ENOENT) {
                spdlog::error("error opening cache file {}: {} ({})", mFilename.c_str(),
                        strerror(errno), errno);
            }
            return;
        } else {
            spdlog::info("{} {}:{} open cache file {} successfully", __FILE__, __FUNCTION__, __LINE__, mFilename.c_str());
        }

        struct stat statBuf;
        if (fstat(fd, &statBuf) == -1) {
            spdlog::error("error stat'ing cache file: {} ({})", strerror(errno), errno);
            close(fd);
            return;
        }

        // Check the size before trying to mmap it.
        size_t fileSize = statBuf.st_size;
        if (fileSize > mMaxTotalSize * 2) {
            spdlog::error("cache file is too large: {}",
                static_cast<off64_t>(statBuf.st_size));
            close(fd);
            return;
        }

        uint8_t* buf = reinterpret_cast<uint8_t*>(mmap(nullptr, fileSize,
                PROT_READ, MAP_PRIVATE, fd, 0));
        if (buf == MAP_FAILED) {
            spdlog::error("error mmaping cache file: {} ({})", strerror(errno),
                    errno);
            close(fd);
            return;
        }

        // Check the file magic and CRC
        size_t cacheSize = fileSize - headerSize;
        if (memcmp(buf, cacheFileMagic, 4) != 0) {
            spdlog::error("cache file has bad mojo");
            close(fd);
            return;
        }
        uint32_t* crc = reinterpret_cast<uint32_t*>(buf + 4);
        if (GenerateCRC32(buf + headerSize, cacheSize) != *crc) {
            spdlog::error("cache file failed CRC check");
            close(fd);
            return;
        }

        int err = unflatten(buf + headerSize, cacheSize);
        if (err < 0) {
            spdlog::error("error reading cache contents: {} ({})", strerror(-err), -err);
            munmap(buf, fileSize);
            close(fd);
            return;
        }

        munmap(buf, fileSize);
        close(fd);
    }
}

void FileBlobCache::writeToFile() {
    // ATRACE_CALL();
    spdlog::info("FileBlobCache::{}", __FUNCTION__);

    if (mFilename.length() > 0) {
        size_t cacheSize = getFlattenedSize();
        size_t headerSize = cacheFileHeaderSize;
        const char* fname = mFilename.c_str();

        // Try to create the file with no permissions so we can write it
        // without anyone trying to read it.
        int fd = open(fname, O_CREAT | O_EXCL | O_RDWR, 0);
        if (fd == -1) {
            if (errno == EEXIST) {
                // The file exists, delete it and try again.
                if (unlink(fname) == -1) {
                    // No point in retrying if the unlink failed.
                    spdlog::error("error unlinking cache file {}: {} ({})", fname,
                            strerror(errno), errno);
                    return;
                }
                // Retry now that we've unlinked the file.
                fd = open(fname, O_CREAT | O_EXCL | O_RDWR, 0);
            }
            if (fd == -1) {
                spdlog::error("error creating cache file {}: {} ({})", fname,
                        strerror(errno), errno);
                return;
            }
        }

        size_t fileSize = headerSize + cacheSize;

        uint8_t* buf = new uint8_t [fileSize];
        if (!buf) {
            spdlog::error("error allocating buffer for cache contents: {} ({})",
                    strerror(errno), errno);
            close(fd);
            unlink(fname);
            return;
        }

        int err = flatten(buf + headerSize, cacheSize);
        if (err < 0) {
            spdlog::error("error writing cache contents: {} ({})", strerror(-err), -err);
            delete [] buf;
            close(fd);
            unlink(fname);
            return;
        }

        // Write the file magic and CRC
        memcpy(buf, cacheFileMagic, 4);
        uint32_t* crc = reinterpret_cast<uint32_t*>(buf + 4);
        *crc = GenerateCRC32(buf + headerSize, cacheSize);

        if (write(fd, buf, fileSize) == -1) {
            spdlog::error("error writing cache file: {} ({})", strerror(errno), errno);
            delete [] buf;
            close(fd);
            unlink(fname);
            return;
        }

        delete [] buf;
        fchmod(fd, S_IRUSR);
        close(fd);
    }
}

size_t FileBlobCache::getSize() {
    if (mFilename.length() > 0) {
        return getFlattenedSize() + cacheFileHeaderSize;
    }
    return 0;
}

}