#include "nvme_passthru.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/nvme_ioctl.h>
#include <fcntl.h>
#include <cstring>
#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <inttypes.h>

using namespace std;

const unsigned int PAGE_SIZE = 4096;
const unsigned int MAX_BUFLEN = 16*1024*1024; /* Maximum transfer size (can be adjusted if needed) */
const unsigned int NSID = 1; /* NSID can be checked using 'sudo nvme list' */
const int DMA_SIZE = 512 * 1024;

int Embedded::Proj1::Open(const std::string &dev) {
    int err;
    err = open(dev.c_str(), O_RDONLY);
    if (err < 0)
        return -1;
    fd_ = err;

    struct stat nvme_stat;
    err = fstat(fd_, &nvme_stat);
    if (err < 0)
        return -1;
    if (!S_ISCHR(nvme_stat.st_mode) && !S_ISBLK(nvme_stat.st_mode))
        return -1;

    return 0;
}

int Embedded::Proj1::ImageWrite(std::vector<uint8_t> &buf) {
    if (buf.empty()) return -EINVAL;
    if (buf.size() > MAX_BUFLEN) {
        cerr << "[ERROR] Image size exceeds the maximum transfer size limit." << endl;
        return -EINVAL;
    }
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */
    int data_len = buf.size();
    int nblocks = data_len / PAGE_SIZE;
    if(data_len % PAGE_SIZE) nblocks++;
    int dma_blocks = DMA_SIZE / PAGE_SIZE;
    int lba = 0;
    int start = 0;
    cout<<"Image size: "<<data_len<<" total blocks: "<<nblocks<<endl;
    while(data_len > 0 && nblocks > 0){
        int trans_len = min(DMA_SIZE, data_len);
        int trans_blocks = min(nblocks, dma_blocks);
        cout<<"DMA_SIZE: "<<min(DMA_SIZE, data_len)<<", NUM_BLOCKS: "<<min(nblocks, dma_blocks)<<endl;
        int res = nvme_passthru(NVME_CMD_WRITE, trans_len, lba, trans_blocks, &buf[start]);
        if(res < 0){
            return -1;
        }
        nblocks -= trans_blocks;
        data_len -= trans_len;
        lba += trans_blocks;
        start += trans_len;
    }
    return 0;
}

int Embedded::Proj1::ImageRead(std::vector<uint8_t> &buf, size_t size) {
    if (size == 0) return -EINVAL;
    if (size > MAX_BUFLEN) {
        cerr << "[ERROR] Requested read size exceeds the maximum transfer size limit." << endl;
        return -EINVAL;
    }
    buf.resize(size);
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */
    int data_len = size;
    int nblocks = data_len / PAGE_SIZE;
    if(data_len % PAGE_SIZE) nblocks++;
    int dma_blocks = DMA_SIZE / PAGE_SIZE;
    int lba = 0;
    int start = 0;
    while(data_len > 0 && nblocks > 0){
        int trans_len = min(DMA_SIZE, data_len);
        int trans_blocks = min(nblocks, dma_blocks);
        cout<<"DMA_SIZE: "<<min(DMA_SIZE, data_len)<<", NUM_BLOCKS: "<<min(nblocks, dma_blocks)<<endl;
        int res = nvme_passthru(NVME_CMD_READ, trans_len, lba, trans_blocks, &buf[start]);
        if(res < 0){
            return -1;
        }
        nblocks -= trans_blocks;
        data_len -= trans_len;
        lba += trans_blocks;
        start += trans_len;
    }
    return 0;
}

int Embedded::Proj1::Hello() {
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */

    return -1; // placeholder
}

int Embedded::Proj1::nvme_passthru(uint8_t opcode, uint32_t data_len, uint64_t lba, uint32_t nblocks, void *addr)
{
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * This function should serve as the low-level interface for issuing
     * passthru NVMe commands. Make sure to include appropriate arguments
     * (e.g., opcode, namespace ID, command dwords, buffer pointer, length,
     * and result field) so that higher-level methods (ImageWrite, ImageRead,
     * Hello) can be implemented using this helper.
     *
     * Hint: refer to the Linux nvme_ioctl.h header and the struct nvme_passthru_cmd definition.
     * - Link: https://elixir.bootlin.com/linux/v5.15/source/include/uapi/linux/nvme_ioctl.h
     * ------------------------------------------------------------------ */
/*
     struct nvme_passthru_cmd {
	__u8	opcode;
	__u8	flags;
	__u16	rsvd1;
	__u32	nsid;
	__u32	cdw2;
	__u32	cdw3;
	__u64	metadata;
	__u64	addr;
	__u32	metadata_len;
	__u32	data_len;
	__u32	cdw10;
	__u32	cdw11;
	__u32	cdw12;
	__u32	cdw13;
	__u32	cdw14;
	__u32	cdw15;
	__u32	timeout_ms;
	__u32	result;
};

*/

    struct nvme_passthru_cmd cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = opcode;
    cmd.nsid = NSID;
    cmd.data_len = data_len;
    cmd.cdw10 = lba & 0xFFFFFFFF;
    cmd.cdw11 = lba >> 32;
    cmd.cdw12 = nblocks & 0x0000FFFF;
    cmd.addr = (__u64)(char*)addr;
    int res = ioctl(fd_, NVME_IOCTL_IO_CMD, &cmd);
    return res;
}

