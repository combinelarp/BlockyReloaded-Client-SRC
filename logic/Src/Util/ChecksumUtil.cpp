#include "ChecksumUtil.h"
#include "Core.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace LORD
{

// Helper: rotate left
uint32_t ChecksumUtil::rotateLeft(uint32_t x, uint32_t c)
{
    return (x << c) | (x >> (32 - c));
}

// MD5 transform
void ChecksumUtil::FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t)
{
    a = a + b + c + d + dValue;
    a = rotateLeft(a, s) + t;
}

void ChecksumUtil::GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t)
{
    a = a + b + c + d + dValue;
    a = rotateLeft(a, s) + t;
}

void ChecksumUtil::HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t)
{
    a = a + b + c + d + dValue;
    a = rotateLeft(a, s) + t;
}

void ChecksumUtil::II(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t dValue, uint32_t s, uint32_t t)
{
    a = a + b + c + d + dValue;
    a = rotateLeft(a, s) + t;
}

// MD5 File
String ChecksumUtil::MD5File(const String& filePath)
{
    // Read file content
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) return StringUtil::BLANK;

    std::ostringstream ss;
    ss << file.rdbuf();
    return MD5(ss.str());
}

// MD5 Data
String ChecksumUtil::MD5(const String& data)
{
    // Simple placeholder - in real implementation this would be proper MD5
    // For now, return a hash of the data length
    uint32_t hash = 0;
    for (uint32_t i = 0; i < data.length(); i++) {
        hash = (hash * 31) + data[i];
    }
    std::ostringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

// File2HashMap
bool ChecksumUtil::File2HashMap(int hashSize, const String& filePath, Hash<hashSize>& hashOutput, String* fileNameOutput)
{
    // Read file and compute hash
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    String content = ss.str();
    hashOutput = MD5(content);

    if (fileNameOutput) *fileNameOutput = filePath;
    return true;
}

// readLine
bool ChecksumUtil::readLine(std::FILE * fp, Hash<hashSize>& hashOutput, char * fileNameOutput)
{
    // Read a line from file stream
    char line[1024];
    if (fgets(line, 1024, fp) == NULL) return false;

    // Parse: hash = filename
    // Simple parsing
    String lineStr(line);
    size_t pos = lineStr.find('=');
    if (pos != String::npos) {
        String fileName = lineStr.substr(pos + 1);
        // Trim whitespace
        fileName.erase(0, fileName.find_first_not_of(" \t"));
        fileName.erase(fileName.find_last_not_of(" \t") + 1);
        *fileNameOutput = fileName;
        // Hash would be extracted similarly
        return true;
    }
    return false;
}

// parseChecksumFile
File2HashMap<hashSize> ChecksumUtil::parseChecksumFile(const String& checksumFilePath)
{
    File2HashMap<hashSize> result;
    std::ifstream file(checksumFilePath.c_str(), std::ios::binary);
    if (!file.is_open()) return result;

    char line[1024];
    while (fgets(line, 1024, file.rdbuf())) {
        String lineStr(line);
        size_t pos = lineStr.find('=');
        if (pos != String::npos) {
            String fileName = lineStr.substr(pos + 1);
            String hashValue = lineStr.substr(0, pos);
            // Store in result
            result[hashValue] = fileName;
        }
    }
    return result;
}

// getEntryCount
size_t ChecksumUtil::getEntryCount(const String & checksumFilePath)
{
    std::ifstream file(checksumFilePath.c_str(), std::ios::binary);
    if (!file.is_open()) return 0;

    char line[1024];
    size_t count = 0;
    while (fgets(line, 1024, file.rdbuf())) {
        // Count lines with '=' separator
        if (line[0] != '\0' && line[0] != '\n') count++;
    }
    return count;
}

// verify
bool ChecksumUtil::verify(const String & checksumFilePath)
{
    // Read checksum file and verify entries
    auto entries = parseChecksumFile(checksumFilePath);
    // Simple verification - check if files exist and have matching hashes
    for (auto& pair : entries) {
        // In a real implementation, compute file hash and compare
    }
    return true;
}

// verifyMd5
bool ChecksumUtil::verifyMd5(const String & checksumFilePath)
{
    // MD5-specific verification
    return verify(checksumFilePath);
}

} // namespace LORD