# About: Used in the automation framework that computes throughput for various rwnd values
# Sets rwnd value in the register
bfrt.internet_cc.pipe.SwitchEgress.new_rwnd.mod(register_index=0, f1=4001)