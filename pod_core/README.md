# pod_core

-m plain -a complaint_pod -p plain_data -o plain_output --demand_ranges 0-10
-m table -a complaint_pod -p table_data -o table_output --demand_ranges 1-20

-m plain -a atomic_swap_pod -p plain_data -o plain_output --demand_ranges 1-10
-m table -a atomic_swap_pod -p table_data -o table_output --demand_ranges 1-10

-m plain -a atomic_swap_pod_vc -p plain_data -o plain_output --demand_ranges 1-10
-m table -a atomic_swap_pod_vc -p table_data -o table_output --demand_ranges 1-10

-m plain -a ot_complaint_pod -p plain_data -o plain_output --demand_ranges 1-2 --phantom_ranges 0-3
-m table -a ot_complaint_pod -p table_data -o table_output --demand_ranges 1-2 --phantom_ranges 0-3

-m table -a vrf_query -p table_data -o table_output -k first_name -v Kathy
-m table -a vrf_query -p table_data -o table_output -k "Emp ID" -v 614227
-m table -a ot_vrf_query -p table_data -o table_output -k "Emp ID" -v 313736 964888 abc -n 350922 aaa eee bbb

--dump_ecc_pub

