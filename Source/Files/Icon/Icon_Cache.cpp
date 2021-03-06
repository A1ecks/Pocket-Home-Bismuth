#include "Icon_Cache.h"
#include <fcntl.h>
#include <unistd.h>

#ifdef JUCE_DEBUG
// Print the full class name before all debug output:
static const constexpr char* dbgPrefix = "Icon::Cache::";
#endif

// Filename shared by all icon cache files:
static const constexpr char* cacheFileName = "/icon-theme.cache";

// Icon file extension flags:

// According to the documentation, these are the correct flag values:
// pngExtensionFlag = 1;
// xpmExtensionFlag = 2;
// svgExtensionFlag = 4;

// However, they always provide incorrect results.
// These values actually work:
static const constexpr juce::uint16 xpmExtensionFlag = 1;
static const constexpr juce::uint16 svgExtensionFlag = 2;
static const constexpr juce::uint16 pngExtensionFlag = 4;

// Maps an icon cache file to memory.
Icon::Cache::Cache(const juce::String& themePath)
{
    using juce::String;
    using juce::File;
    using juce::uint32;
    String cachePath = themePath + cacheFileName;
    File cacheFile(cachePath);
    if (!cacheFile.existsAsFile())
    {
        DBG(dbgPrefix << __func__ << ": Failed to find cache file "
                << cachePath);
        return;
    }

    fileLen = cacheFile.getSize();
    if (fileLen <= 0)
    {
        DBG(dbgPrefix << __func__ << ": Cache file is empty, path = "
                << cachePath);
        return;
    }

    if (cacheFile.getLastModificationTime().toMilliseconds() <
        cacheFile.getParentDirectory().getLastModificationTime()
        .toMilliseconds())
    {
        DBG(dbgPrefix << __func__ << ": Cache file is out of date, path = "
                << cachePath);
        return;
    }

    fd = open(cachePath.toRawUTF8(), O_RDONLY, 0);
    if (fd <= 0)
    {
        DBG(dbgPrefix << __func__ << ": Failed to open cache file, path = "
                << cachePath);
        return;
    }
    fileMap = mmap(nullptr, fileLen, PROT_READ, MAP_PRIVATE | MAP_POPULATE,
            fd, 0);
    if (fileMap == MAP_FAILED)
    {
        DBG(dbgPrefix << __func__ << ": Failed to map cache file, path = "
                << cachePath);
        return;
    }

    // Quick method for determining local byte order from
    // https://stackoverflow.com/a/1001330

    // Checking byte order on runtime is slightly more convenient than checking
    // during compilation, and the cost of checking this when each icon cache
    // object is created is insignificant.
    bigEndian = (htonl(47) == 47);

    uint32 hashOffset = read32(4);
    uint32 dirListOffset = read32(8);

    // Read in the list of directory names:
    uint32 numDirs = read32(dirListOffset);
    dirListOffset += 4;
    for (int i = 0; i < numDirs; i++)
    {
        uint32 offset = read32(dirListOffset + i * 4);
        String dirPath = readString(offset);
        directories.add(dirPath);
    }

    // Read in the list of hash bucket offsets:
    hashBuckets = read32(hashOffset);
    hashOffset += 4;
    for (int i = 0; i < hashBuckets; i++)
    {
        hashOffsets.add(read32(hashOffset + i * 4));
    }
}


// Closes the cache file on destruction.
Icon::Cache::~Cache()
{
    if (fileMap != MAP_FAILED)
    {
        int rc = munmap(fileMap, fileLen);
        jassert(rc == 0);
        fileMap = MAP_FAILED;
    }
    if (fd > 0)
    {
        close(fd);
        fd = 0;
    }
}


// Checks if this object represents a valid cache file. This ensures that the
// cache file exists, is not out of date, and contains icon data.
bool Icon::Cache::isValidCache() const
{
    return fileMap != MAP_FAILED && fileLen > 0
            && !directories.isEmpty() && !hashOffsets.isEmpty();
}


// Looks up an icon's data in the icon cache.
std::map<juce::String, juce::String> Icon::Cache::lookupIcon
(const juce::String& iconName) const
{
    using juce::String;
    using juce::uint32;
    std::map<String, String>  matches;
    if (!isValidCache())
    {
        return matches;
    }
    uint32 hashVal = hashValue(iconName.toRawUTF8());
    for (uint32 offset = hashOffsets[hashVal];
        offset > 0 && offset < fileLen;
        offset = read32(offset))
    {
        String name = readString(read32(offset + 4));
        if (iconName == name)
        {
            uint32 imageOffset = read32(offset + 8);
            uint32 numImg = read32(imageOffset);
            imageOffset += 4;
            for (uint32 img = imageOffset;
                img < (imageOffset + numImg * 8);
                img += 8)
            {
                juce::uint16 iconFlags = read16(img + 2);
                String extension;
                if ((iconFlags & pngExtensionFlag) == pngExtensionFlag)
                {
                    extension = ".png";
                }
                else if ((iconFlags & xpmExtensionFlag) == xpmExtensionFlag)
                {
                    extension = ".xpm";
                }
                else if ((iconFlags & svgExtensionFlag) == svgExtensionFlag)
                {
                    extension = ".svg";
                }
                if (extension.isNotEmpty())
                {
                    matches[directories[read16(img)]] = extension;
                }
            }
            break;
        }
    }
    return matches;
}


// Calculate the hash value of an icon name.
juce::uint32 Icon::Cache::hashValue(const char* icon) const
{
    if (icon == nullptr)
    {
        jassertfalse;
        return 0;
    }
    juce::uint32 val = (int) * icon;
    for (const char* ch = icon + 1; *ch != '\0'; ch++)
    {
        val = (val << 5) - val + *ch;
    }
    return val % hashBuckets;
}


// Reads a two-byte unsigned integer from an arbitrary offset in the cache file,
// after ensuring that the cache file is valid and the offset is within the file
// bounds.
juce::uint16 Icon::Cache::read16(juce::uint32 offset) const
{
    using juce::uint16;
    if (fileMap == MAP_FAILED || (offset + sizeof(uint16)) > fileLen
        || (offset + sizeof(uint16)) < offset)
    {
        jassertfalse;
        return 0;
    }
    uint16 value = * reinterpret_cast<uint16*>
            (reinterpret_cast<char*>(fileMap) + offset);
    return bigEndian ? value : htons(value);
}


// Reads a four-byte unsigned integer from an arbitrary offset in the cache
// file, after ensuring that the cache file is valid and the offset is within
// the file bounds.
juce::uint32 Icon::Cache::read32(juce::uint32 offset) const
{
    using juce::uint32;
    if (fileMap == MAP_FAILED || (offset + sizeof(uint32)) > fileLen
        || (offset + sizeof(uint32)) < offset)
    {
        jassertfalse;
        return 0;
    }
    uint32 value = * reinterpret_cast<uint32*>
            (reinterpret_cast<char*>(fileMap) + offset);
    return bigEndian ? value : htonl(value);
}


// Reads a null-terminated character string from an arbitrary offset in the
// cache file, after ensuring that the cache file is valid and the offset is
// within the file bounds.
juce::String Icon::Cache::readString(juce::uint32 offset) const
{
    using juce::String;
    using juce::CharPointer_UTF8;
    if (fileMap == MAP_FAILED || offset >= fileLen)
    {
        jassertfalse;
        return String();
    }
    int length = 0;
    while (* (reinterpret_cast<char*>(fileMap) + offset + length) != '\0')
    {
        length++;
        if ((length + offset) >= fileLen)
        {
            jassertfalse;
            return String();
        }
    }
    return String(CharPointer_UTF8((char*) fileMap + offset));
}
