# cnpm
c++ package manager

Managing projects with package.json and also dependencies that don't have a package.json.

## Start
Need gcc 12.0.0 or later

```
bootstrap.sh
```
After initial build , this project can build itself though eventually I hope to distribute to
linux and mac (maybe windows).

## Repeat build while developing this project

```
make -j3
```

Usage is from help

```
./cnpm -h
```

This will work with any language and is most useful where there are dependent projects without packages.


And we should be able to get speed from the modern use of c++ and eventually make it parallel.

The packages:

https://github.com/libgit2/libgit2.git
https://github.com/openssl/openssl\#OpenSSL_1_1_1l.git

are core to this project and do the heavy url lifting
