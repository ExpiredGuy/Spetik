#include "serialize.h"
#include <fstream>

namespace NNUE {

bool Serializer::save(const Network& network, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    
    // Write header
    file.write(reinterpret_cast<const char*>(&kFileMagic), sizeof(kFileMagic));
    file.write(reinterpret_cast<const char*>(&kVersion), sizeof(kVersion));
    
    // Write network data
    file.write(reinterpret_cast<const char*>(network.feature_weights), 
              sizeof(network.feature_weights));
    // Write other layers...
    
    return file.good();
}

bool Serializer::load(Network& network, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    
    // Verify header
    uint32_t magic;
    uint16_t version;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    if (magic != kFileMagic || version != kVersion) return false;
    
    // Read network data
    file.read(reinterpret_cast<char*>(network.feature_weights), 
             sizeof(network.feature_weights));
    // Read other layers...
    
    return file.good();
}

} // namespace NNUE