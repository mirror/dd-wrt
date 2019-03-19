## Requirements

* cmake >= 2.8.12
* swig >= 3.0.0
* node >= 0.10.x < 4.9.0
* npm
* libpcre

## Install

```
$ cmake -DENABLE_STATIC=ON -DGEN_LANGUAGE_BINDINGS=ON -DGEN_JAVASCRIPT_BINDINGS=ON ..
$ make
$ make install #necessary for plugin's
$ cd swig/javascript
$ npm install --unsafe-perm
```

## Test

```
$ node test.js
```

## SWIG

Current SWIG version does not support shared pointer. To correct that at the moment you need to do one thing:

1) Use the latest SWIG master branch with the pull request [Add shared pointer support in javascript](https://github.com/swig/swig/pull/236).

You can build the JavaScript bindings inside docker, for that you can use the [dockerfile](./Dockerfile), it is based on the [dockerfile](https://github.com/sysrepo/sysrepo/blob/master/deploy/docker/sysrepo-netopeer2/platforms/Dockerfile.arch.devel).

More details on SWIG's JavaScript bindings can be found at [SWIG](http://www.swig.org/Doc3.0/Javascript.html#Javascript_node_extensions).

## Usage

To include the node bindings simply use it with.

```
var yang = require("./index")
```
