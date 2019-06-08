# pod_core

-e ecc_pub.bin -m plain -a batch_pod -p plain_data -o plain_output --demand_ranges 0-10
-e ecc_pub.bin -m table -a batch_pod -p table_data -o table_output --demand_ranges 1-20

-e ecc_pub.bin -m plain -a batch2_pod -p plain_data -o plain_output --demand_ranges 1-10
-e ecc_pub.bin -m table -a batch2_pod -p table_data -o table_output --demand_ranges 1-10

-e ecc_pub.bin -m plain -a ot_batch_pod -p plain_data -o plain_output --demand_ranges 1-2 --phantom_ranges 0-3
-e ecc_pub.bin -m table -a ot_batch_pod -p table_data -o table_output --demand_ranges 1-2 --phantom_ranges 0-3

-e ecc_pub.bin -m plain -a batch3_pod -p plain_data -o plain_output --demand_ranges 1-10
-e ecc_pub.bin -m table -a batch3_pod -p table_data -o table_output --demand_ranges 1-8000

-e ecc_pub.bin -m table -a vrf_query -p table_data -o table_output -k first_name -v Kathy
-e ecc_pub.bin -m table -a vrf_query -p table_data -o table_output -k "Emp ID" -v 614227
-e ecc_pub.bin -m table -a ot_vrf_query -p table_data -o table_output -k "Emp ID" -v 313736 964888 abc -n 350922 aaa eee bbb

-e ecc_pub.bin --dump_ecc_pub

