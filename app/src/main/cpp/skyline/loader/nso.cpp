#include <lz4.h>
#include <vector>
#include "nso.h"

namespace skyline::loader {
    NsoLoader::NsoLoader(const int romFd) : Loader(romFd) {
        ReadOffset((u32 *) &header, 0x0, sizeof(NsoHeader));
        if (header.magic != constant::NsoMagic)
            throw exception("Invalid NSO magic! 0x{0:X}", header.magic);
    }

    void NsoLoader::LoadProcessData(const std::shared_ptr<kernel::type::KProcess> process, const DeviceState &state) {
        std::vector<u8> textCompressed(header.textCompressedSize), text(header.text.decompressedSize);
        std::vector<u8> rodataCompressed(header.roCompressedSize), rodata(header.ro.decompressedSize);
        std::vector<u8> dataCompressed(header.dataCompressedSize), data(header.data.decompressedSize);

        ReadOffset(textCompressed.data(), header.text.fileOffset, header.textCompressedSize);
        ReadOffset(rodataCompressed.data(), header.ro.fileOffset, header.roCompressedSize);
        ReadOffset(dataCompressed.data(), header.data.fileOffset, header.dataCompressedSize);

        LZ4_decompress_safe(reinterpret_cast<char*>(textCompressed.data()), reinterpret_cast<char*>(text.data()), textCompressed.size(), text.size());
        LZ4_decompress_safe(reinterpret_cast<char*>(rodataCompressed.data()), reinterpret_cast<char*>(rodata.data()), rodataCompressed.size(), rodata.size());
        LZ4_decompress_safe(reinterpret_cast<char*>(dataCompressed.data()), reinterpret_cast<char*>(data.data()), dataCompressed.size(), data.size());

        PatchCode(text);

        u64 textSize = header.text.decompressedSize;
        u64 rodataSize = header.ro.decompressedSize;
        u64 dataSize = header.data.decompressedSize + header.bssSize;

        process->MapPrivateRegion(constant::BaseAddr + header.text.memoryOffset, textSize, {true, true, true}, memory::Type::CodeStatic, memory::Region::Text); // R-X
        state.logger->Debug("Successfully mapped region .text @ 0x{0:X}, Size = 0x{1:X}", constant::BaseAddr + header.text.memoryOffset, textSize);

        process->MapPrivateRegion(constant::BaseAddr + header.ro.memoryOffset, rodataSize, {true, false, false}, memory::Type::CodeReadOnly, memory::Region::RoData); // R--
        state.logger->Debug("Successfully mapped region .ro @ 0x{0:X}, Size = 0x{1:X}", constant::BaseAddr + header.ro.memoryOffset, rodataSize);

        process->MapPrivateRegion(constant::BaseAddr + header.data.memoryOffset, dataSize, {true, true, false}, memory::Type::CodeStatic, memory::Region::Data); // RW-
        state.logger->Debug("Successfully mapped region .data @ 0x{0:X}, Size = 0x{1:X}", constant::BaseAddr + header.data.memoryOffset, dataSize);

        process->WriteMemory(text.data(), constant::BaseAddr + header.text.memoryOffset, textSize);
        process->WriteMemory(rodata.data(), constant::BaseAddr + header.ro.memoryOffset, rodataSize);
        process->WriteMemory(data.data(), constant::BaseAddr + header.data.memoryOffset, dataSize);
    }
}
