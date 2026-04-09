#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Embedded {
    enum nvme_opcode {
        NVME_CMD_WRITE                  = 0x01,
        NVME_CMD_READ                   = 0x02,
        NVME_CMD_SELF_INTRO             = 0x03,
        /* Additional opcodes may be defined here */ 
    };

    class Proj1 {
        public:
            Proj1() : fd_(-1) {}
            int Open(const std::string &dev);
            int ImageWrite(std::vector<uint8_t> &buf);
            int ImageRead(std::vector<uint8_t> &buf, size_t size);
            int Hello();
        private:
            int fd_;
            int nvme_passthru(uint8_t opcode, uint32_t data_len, uint64_t lba, uint32_t nblocks, void *addr);
    };
}
