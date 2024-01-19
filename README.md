sudo ./check-sm-table 0x7fff888001230000 0x143 0x86 0x0 0x0 0x1afcce000  
sudo ./check-sm-table [input_guest_addr] [input_pasid_val] [input_bus_num] [input_dev_num] [input_func_num] [input_rte_addr]  
input_psid_val=0 means used rid_pasid on pasid directory entry, if no rid_pasid, the checker would print"input pasid is zero and no rid_pasid"
