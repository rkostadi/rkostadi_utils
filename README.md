# Quick install
```
# pre-requisites
# compiler that supports c++11
# On Fedora:
dnf install gsl gsl-devel

# To compile
make all
```
# Examples
Sampling at random 500,000 integers from a vector of 1 million integers.
```
cd bin
./sample_at_random -b 1000000 -n 500000 > test1.txt
```
