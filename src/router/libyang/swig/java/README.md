## Requirements

* cmake >= 2.8.12
* swig
* java

## Build And Install
make sure you are in libyang-src-dir/build directory
```
$ cmake -DGEN_LANGUAGE_BINDINGS=ON -DGEN_JAVA_BINDINGS=ON ..
$ make
$ make install
```

## Test
if you wang to test, make sure you are in libyang-src-dir/build
```
$ cmake -DGEN_LANGUAGE_BINDINGS=ON -DGEN_JAVA_BINDINGS=ON -DENABLE_BUILD_TESTS=ON ..
$ make
$ cd swig/java && make test
```