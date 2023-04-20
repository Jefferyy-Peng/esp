# Copyright (c) 2011-2023 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>
set_directive_interface -mode ap_none "top" conf_info_Train_epochs
set_directive_interface -mode ap_none "top" conf_info_features
set_directive_interface -mode ap_none "top" conf_info_classes
set_directive_interface -mode ap_none "top" conf_info_windows
set_directive_interface -mode ap_none "top" conf_info_Test_epochs
set_directive_interface -mode ap_none "top" conf_info_Tol

# Insert here any custom directive
set_directive_dataflow "top/go"
