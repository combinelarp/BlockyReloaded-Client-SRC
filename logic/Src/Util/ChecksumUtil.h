#pragma once

#include "Core.h"
#include "Util/PathUtil.h"
#include <cstdint>

namespace LORD
{

class LORD_CORE_API ChecksumUtil
{
public:
    static bool File2HashMap(int hashSize, const String& filePath, Hash<hashSize>& hashOutput, String* fileNameOutput = NULL);
    static bool readLine(std::FILE * fp, Hash<hashSize>& hashOutput, char * fileNameOutput);
    static File2HashMap<hashSize> parseChecksumFile(const String& checksumFilePath);
    static size_t getEntryCount(const String & checksumFilePath);
    static bool verify(const String & checksumFilePath);
    static bool verifyMd5(const String & checksumFilePath);

    // MD5 helpers
    static String MD5(const String& data);
    static String MD5File(const String& filePath);

private:
    // Internal MD5 implementation
    static uint32_t rotateLeft(uint32_t x, uint32_t c);
    static void FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t);
    static void GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t);
    static void HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t);
    static void II(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t);

    static unsigned char padding[64];
    static unsigned char state[4];
    static unsigned char count[2];
    static unsigned char digest[16];
};

} // namespace LORD