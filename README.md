# zkPoD-lib

zkPoD-lib is the underlying core library for [zkPoD system](https://github.com/sec-bit/zkPoD-node). It fully implements PoD (proof of delivery) protocol and also provides CLI interface together with Golang bindings.

zkPoD-lib has four main parts as followed.

- [`pod_setup`](pod_setup/) - Generates public parameters of system
- [`pod_publish`](pod_publish/) - Publish utility to preprocess data and calculate authenticators
- [`pod_core`](pod_core/) - Core implementations of different zkPoD trading features
- [`pod-go`](pod-go/) - Golang bindings for `pod_setup` and `pod_core` functions

## Related zkPoD projects

- [zkPoD-node](https://github.com/sec-bit/zkPoD-node) Node application written in Golang for buyers (A) and sellers (B). It deals with communication, smart contract calling, data transferring, and other zkPoD protocol interactions.
- [zkPoD-contract](https://github.com/sec-bit/zkPoD-contract) Smart contracts for zkPoD *Decentralized Exchange*.

## Dependencies

- g++ 8.0
- Go 1.11
- Boost 1.69.0 or newer
- OpenMP 5.3.1 or newer

```shell
# For Ubuntu
sudo apt-get install libomp-dev
sudo apt-get install libcrypto++-dev
sudo apt-get install libboost-all-dev
sudo apt-get install libgmp-dev
```

## Build

```shell
mkdir zkPoD && cd zkPoD
git clone zkPoD-lib
cd zkPoD-lib
make

# These files should be generated after successful build.
# pod_publish/pod_setup
# pod_publish/pod_publish
# pod_core/pod_core
# pod_core/libpod_core.so
```

## Usage

### pod_setup

```shell
$ ./pod_setup -h

command line options:
  -h [ --help ]                    Use -h or --help to list all arguments
  -o [ --output ] arg              Provide the output ecc pub file
  -a [ --u1_size ] arg (=1026)     Provide the max number of u1
  -b [ --u2_size ] arg (=2)        Provide the max number of u2
  -t [ --omp_thread_num ] arg (=0) Provide the number of the openmp thread, 1:
                                   disable openmp, 0: default.
```

Use `pod_setup` to generate public parameters.

```shell
./pod_setup -o ecc_pub.bin
```

`ecc_pub.bin` is generated after successful setup.

### pod_publish


```shell
$ ./pod_publish -h
command line options:
  -h [ --help ]                         Use -h or --help to list all arguments
  ---e ecc_pub_file -m table -f file -o output_path -t table_type -k keys
                                        publish table file
  ---e ecc_pub_file -m plain -f file -o output_path -c column_num
                                        publish plain file
  -e [ --ecc_pub_file ] arg             Provide the ecc pub file
  -m [ --mode ] arg (=plain)            Provide pod mode (plain, table)
  -f [ --publish_file ] arg             Provide the file which want to publish
  -o [ --output_path ] arg              Provide the publish path
  -t [ --table_type ] arg (=csv)        Provide the publish file type in table
                                        mode (csv)
  -c [ --column_num ] arg (=1023)       Provide the column number per
                                        block(line) in plain mode (default
                                        1023)
  -k [ --vrf_colnum_index ] arg         Provide the publish file vrf key column
                                        index positions in table mode (for
                                        example: -k 0 1 3)
  -u [ --unique_key ] arg               Provide the flag if publish must unique
                                        the key in table mode (for example: -u
                                        1 0 1)
  --omp_thread_num arg (=0)             Provide the number of the openmp
                                        thread, 1: disable openmp, 0: default.
```

Use `pod_publish` to preprocess target data. 

zkPoD supports two modes: binary mode and table mode. 

+ binary mode
+ table mode (CSV files)

```shell
# binary mode
./pod_publish -e ecc_pub.bin -m plain -f test.txt -o plain_data -c 1024

# table mode
./pod_publish -e ecc_pub.bin -m table -f test1000.csv -o table_data -t csv -k 0 1
```

Check output folder after publish.

### pod_core

```shell
$ ./pod_core -h
command line options:
  -h [ --help ]             Use -h or --help to list all arguments
  -e [ --ecc_pub_file ] arg Provide the ecc pub file
  -m [ --mode ] arg         Provide pod mode (plain, table)
  -a [ --action ] arg       Provide action (range_pod, ot_range_pod, vrf_query,
                            ot_vrf_query...)
  -p [ --publish_path ] arg Provide the publish path
  -o [ --output_path ] arg  Provide the output path
  --demand_ranges arg       Provide the demand ranges
  --phantom_ranges arg      Provide the phantom range(plain mode)
  -k [ --query_key ] arg    Provide the query key name(table mode)
  -v [ --key_value ] arg    Provide the query key values(table mode, for
                            example -v value_a value_b value_c)
  -n [ --phantom_key ] arg  Provide the query key phantoms(table mode, for
                            example -n phantoms_a phantoms_b phantoms_c)
  --omp_thread_num arg      Provide the number of the openmp thread, 1: disable
                            openmp, 0: default.
  -c [ --use_c_api ]
  --test_evil
  --dump_ecc_pub
```

`pod_core` supports several mode combination. Basically, we have `atomic-swap` and `complaint` trade mode for binary and table files. Moreover, we could employ *oblivious transfer*, `OT` mode, for privacy-preserving download. Furthermore, we are allowed to do `vrf_query` of structured table data, which could be combined with `OT` mode for private query.

```shell
./pod_core -e ecc_pub.bin -m plain -a batch_pod -p plain_data -o plain_output --demand_ranges 0-2

./pod_core -e ecc_pub.bin -m table -a ot_vrf_query -p table_data -o table_output -k "Emp ID" -v 313736 964888 abc -n 350922 aaa eee bbb
```

Check [here](pod_core/README.md) for more CLI interface examples. You could look over [each](pod_core/scheme_batch_test.cc) [test](pod_core/scheme_batch2_test.cc) for detailed protocol implementation.

### pod-go

A simple Golang wrapper for zkPoD-lib is provided for easier library integration.

```shell
cd pod-go
export GO111MODULE=on
make test
```

All tests should pass as expected. You could check over [tests](pod-go/plain/batch/api_test.go) to learn about usage.

## License

zkPoD-lib is released under the terms of the MIT license. See LICENSE for more information or see https://opensource.org/licenses/MIT.