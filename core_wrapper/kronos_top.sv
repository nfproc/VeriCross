// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************

// Kronos Wrapper Module for VeriCross
module kronos_top (
  input  logic        clk,
  input  logic        rst_x,
  // instruction logging
  output logic        inst_valid,
  output logic [31:0] inst_pc,
  output logic [31:0] inst_ir,
  output logic [4:0]  inst_rd,
  output logic [31:0] inst_result,
  output logic [31:0] inst_d_addr,
  output logic [31:0] inst_d_wdata,
  output logic [3:0]  inst_d_wmask,
  output logic [1:0]  inst_d_baddr,
  // instruction memory
  output logic [31:0] mem_i_addr,
  input  logic [31:0] mem_i_rdata,
  // data memory
  output logic [31:0] mem_d_addr,
  input  logic [31:0] mem_d_rdata,
  output logic [31:0] mem_d_wdata,
  output logic [3:0]  mem_d_wmask,
  output logic        mem_en,
  output logic        mem_d_re,
  output logic        mem_d_we
);

  logic mem_i_en, mem_i_ack, mem_d_en, mem_d_ack;

  kronos_core #(.BOOT_ADDR('h00010000))
  u_dut (
    .clk               (clk         ),
    .rstz              (rst_x       ),
    .instr_addr        (mem_i_addr  ),
    .instr_data        (mem_i_rdata ),
    .instr_req         (mem_i_en    ),
    .instr_ack         (mem_i_ack   ),
    .data_addr         (mem_d_addr  ),
    .data_rd_data      (mem_d_rdata ),
    .data_wr_data      (mem_d_wdata ),
    .data_mask         (mem_d_wmask ),
    .data_wr_en        (mem_d_we    ),
    .data_req          (mem_d_en    ),
    .data_ack          (mem_d_ack   ),
    .software_interrupt(1'b0        ),
    .timer_interrupt   (1'b0        ),
    .external_interrupt(1'b0        )
  );

  // default testbench of kronos assumes a single-port RAM
  // so, request to either instruction or data memory will be accepted
  assign mem_en    = mem_i_en | mem_d_en;
  assign mem_d_re  = mem_d_en & (~ mem_d_we);
  
  always_ff @(posedge clk) begin
    mem_i_ack <= mem_i_en & (~ mem_d_en);
    mem_d_ack <= mem_d_en;
  end

  // information for instruction logging is extracted from EX stage
  assign inst_valid   = u_dut.u_ex.instr_vld;
  assign inst_pc      = u_dut.u_ex.decode.pc;
  assign inst_ir      = u_dut.u_ex.decode.ir;
  assign inst_rd      = u_dut.u_ex.rd;
  assign inst_result  = u_dut.u_ex.result;
  assign inst_d_addr  = mem_d_addr;
  assign inst_d_wdata = mem_d_wdata;
  assign inst_d_wmask = mem_d_wmask;
  assign inst_d_baddr = u_dut.u_ex.u_lsu.byte_addr;

endmodule

// *********************************************************************************************