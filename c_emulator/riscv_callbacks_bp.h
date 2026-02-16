#pragma once
#include "sail.h"
#include "riscv_callbacks_if.h"

class bp_callbacks : public callbacks_if {

public:
  bp_callbacks(uint64_t breakpoint, uint64_t memstart, uint64_t memlen);

  void pc_write_callback(ModelImpl &model, sbits new_pc) override;

private:

  void dump_mem_to_file();

  uint64_t breakpoint;
  bool first_done;

  uint64_t memstart;
  uint64_t memlen;
};
