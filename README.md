cachesim
========
A program designed to simulate a single-level, set-associative, LRU cache with a write-back and write-allocate write policy.

Building
--------
To build `cachesim`, simply run `make all`. Alternatively, if you need to generate full debug information, then use `make debug`.

Running
-------
After building, run with the following parameters:

```shell
./cachesim <tracefile> <cache-size> <n-way-associativity> <block-size>
```

Tracefile
---------
The tracefile should contain store and load instructions in the following format:

```
store <address in hex> <access size in bytes> <value in hex>

load <address in hex> <access size>
```

For example:

```
store 0x1234ab00 2 19ab

load 0x002a173f 4
```

Contributors
------------
Kevin Gao [kag45]

Oliver Fang [orf2]
