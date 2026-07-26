// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************

// RVCoreP Wrapper Module for VeriCross
module rvcorep_main (
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

  logic [31:0] Ex_D_ADDR, ExMa_D_ADDR;
  logic [3:0]  Ex_D_WE, ExMa_D_WE;

  RVCore p(
    .CLK    (clk),
    .RST_X  (rst_x), // SYNCHRONOUS reset (active low)
    .r_rout (),
    .r_halt (),
    .I_ADDR (mem_i_addr),
    .D_ADDR (Ex_D_ADDR),
    .I_IN   (mem_i_rdata),
    .D_IN   (mem_d_rdata),
    .D_OUT  (mem_d_wdata),
    .D_WE   (Ex_D_WE),
    .D_RE   (mem_d_re),
    .D_STALL(1'b0)
  );

  assign inst_valid   = p.ExMa_v;
  assign inst_pc      = {{14{1'b0}}, p.ExMa_pc};
  assign inst_ir      = p.ExMa_ir;
  assign inst_rd      = p.ExMa_rd;
  assign inst_result  = p.Ma_rslt;
  assign inst_d_addr  = ExMa_D_ADDR;
  assign inst_d_wdata = p.ExMa_wdata;
  assign inst_d_wmask = ExMa_D_WE;
  assign inst_d_baddr = p.ExMa_addr;
  
  assign mem_d_addr   = {Ex_D_ADDR[31:2], 2'b00};
  assign mem_d_wmask  = Ex_D_WE;
  assign mem_en       = 1'b1;
  assign mem_d_we     = |mem_d_wmask;

  always_ff @ (posedge clk) begin
    if (! rst_x) begin
      ExMa_D_ADDR <= 0;
      ExMa_D_WE   <= 4'h0;
    end else begin
      ExMa_D_ADDR <= {Ex_D_ADDR[31:2], 2'b00};
      ExMa_D_WE   <= Ex_D_WE;
    end
  end
endmodule

// *********************************************************************************************