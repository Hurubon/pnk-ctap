# What is pnk-ctap?
Pnk-ctap is a compile time (command-line) argument parser library. It works by representing each argument as a distinct
type and storing them in a type set. The set is constructed at compile-time, meaning the only runtime cost is parsing the
arguments and storing them in the set. 

The library is written in a literary programming style so that new users can quickly learn how to use it and so that
C++ programers of almost any level can understand the code and learn from the ideas used in it.

Pnk-ctap was developed with the help and inspired by the work of [wreien](https://github.com/wreien), 
[karnkaul](https://github.com/karnkaul), [eisenwave](https://github.com/Eisenwave) and others from the [Together C & C++
Discord community](https://discord.gg/tccpp).

# How can I benefit from using pnk-ctap?
With pnk-ctap, you can create apps with expressive command line interfaces while having zero overhead, type safety, extensibility
and clean code. Include one header and start seeing the benefits.
