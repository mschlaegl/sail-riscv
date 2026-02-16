#include <string>
#include <iomanip>
#include <cstring>

#include "riscv_callbacks_bp.h"

struct block {
    uint64_t block_id;
    unsigned char *mem;
    struct block *next;
};

extern struct block *sail_memory;
extern uint64_t MASK;

bp_callbacks::bp_callbacks(uint64_t breakpoint, uint64_t memstart, uint64_t memlen)
  : breakpoint(breakpoint),
    memstart(memstart),
    memlen(memlen){}


// Implementations of default callbacks for breakpoints.
// The model assumes that these functions do not change the state of the model.

void bp_callbacks::pc_write_callback(ModelImpl &model, sbits new_pc) {
  if (new_pc.bits == breakpoint) {
    dump_mem_to_file();

    model.model_fini();

	exit(EXIT_SUCCESS);
  }
}

void bp_callbacks::dump_mem_to_file() {

  std::stringstream stream;
  stream << "./mem.0x" <<  std::hex << memstart << ".bin";
  std::string filename(stream.str());

  FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
      fprintf(stderr, "Error: Could not open file %s for writing\n", filename.c_str());
        return;
    }
    //printf("Dumping memory from 0x%lx to 0x%lx (%lu bytes) to %s\n",
    //       memstart, memstart + memlen, memlen, filename.c_str());

    uint64_t block_size = MASK + 1;
    uint64_t current_addr = memstart;
    uint64_t end_addr = memstart + memlen;

    while (current_addr < end_addr) {
        uint64_t mask = current_addr & ~MASK;
        uint64_t block_offset = current_addr & MASK;
        uint64_t bytes_remaining_in_block = block_size - block_offset;
        uint64_t bytes_to_write = (current_addr + bytes_remaining_in_block <= end_addr) ?
                                   bytes_remaining_in_block : (end_addr - current_addr);

        // Find the block containing this address
        struct block *blk = sail_memory;
        bool found = false;

        while (blk != NULL) {
            if (blk->block_id == mask) {
                // Found the block - write the data from this block
                fwrite(&blk->mem[block_offset], 1, bytes_to_write, fp);
                found = true;
                break;
            }
            blk = blk->next;
        }

        if (!found) {
            // Block not allocated - write zeros
            static uint8_t zeros[65536];  // 64KB zero buffer
            memset(zeros, 0, sizeof(zeros));
            uint64_t remaining = bytes_to_write;
            while (remaining > 0) {
                uint64_t chunk = (remaining > sizeof(zeros)) ? sizeof(zeros) : remaining;
                fwrite(zeros, 1, chunk, fp);
                remaining -= chunk;
            }
        }

        current_addr += bytes_to_write;
    }

    fclose(fp);
    //printf("Memory dump complete: %s\n", filename.c_str());
}
