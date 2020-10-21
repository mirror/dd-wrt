## Requirements

* cmake >= 2.8.12
* swig
* python3-dev (for python3 bindings)
* python-dev (for python2 bindings)

## Install

```
$ cd libyang/build
$ cmake -DGEN_LANGUAGE_BINDINGS=ON -DGEN_CPP_BINDINGS=ON -DGEN_PYTHON_BINDINGS=ON ..
$ # for python2 bindings add -DGEN_PYTHON_VERSION=2
$ make
$ make install
```

## Recommendations

To avoid problems it is recommended to use separate build directories for Python 2 and Python 3 bindings.
