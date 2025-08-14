#pragma once
#include <string>
#include "architecture.h"

namespace NNUE {

class Serializer {
public:
    static bool save(const Network& network, const std::string& filename);
    static bool load(Network& network, const std::string& filename);
    
private:
    static constexpr uint32_t kFileMagic = 0x4E4E5545; // 'NNUE'
    static constexpr uint16_t kVersion = 1;
};

} // namespace NNUE