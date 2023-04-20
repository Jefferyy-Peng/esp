############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project proj_svd
set_top svd_top
add_files svd.cpp
add_files -tb svd_tb.cpp
open_solution "solution1"
set_part {xc7k160tfbg484-1}
create_clock -period 4 -name default
source "./proj_svd/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
