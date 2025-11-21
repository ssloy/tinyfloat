# TinyFloat - the most unoptimized soft float library on the net

This is a software 32-bit floating point implementation.
While there are plenty soft floating point libraries in the wild, I am not satisfied with either of those.
All of them are way too complex and pretty much undocumented.

**Despite the focus on readability, there are very few comments in the code. An extensive writing is available here:**

1. [Floating point myths and superstitions](https://haqr.eu/tinyfloat/)
2. [Computers and numbers](https://haqr.eu/tinyfloat/computers-and-numbers/)
3. [Printing floats](https://haqr.eu/tinyfloat/print/)
4. [Summation and numerical errors](https://haqr.eu/tinyfloat/add/)
5. [Division and multiplication (work in progress)](https://haqr.eu/tinyfloat/div/)

This is a tiny project, the goal is to make a **simple and understandable** implementation of single-precision floating point.
I do not care about efficiency as long as the code is simple.
The goal is twofold:

* first, educate on how floating point works
* second, provdide the code that can be ported to platforms without floating point.
  
For example, I used it to create an eyecandy raytracer for my [TinyCompiler project](https://haqr.eu/tinycompiler/).

## Someone is wrong on the internet!

![](https://haqr.eu/tinyfloat/home/bad-news.jpg)

Recently, I needed to emulate floating point operations using only integer arithmetic, since floats were unavailable.
I went online to look for a ready-made library and to my surprise, not only did I not find what I was looking for, but I also discovered that someone on the internet was wrong. :)

It turns out that forums are full of people who don't fully understand how computers manipulate numbers.
For example, I pulled the above meme from Reddit (it was me who crossed it out).
Someone was so scared of the terrible rounding errors of floating point numbers that he even made a funny picture.
The only problem is that **0.5 + 0.5 is exactly equal to 1.0**.

So, I decided to roll up my sleeves and reinvent the wheel.
That is, to write the most unoptimized C++ library for emulating IEEE754 32-bit floating-point numbers using only 32-bit integer arithmetic.

## Compilation & testing

There are two kinds of tests in the project: [unit tests](https://github.com/ssloy/tinyfloat/tree/main/tests) where TinyFloat functions are tested against C++ native `float`.
There is also a [truncated version](https://github.com/ssloy/tinyfloat/blob/main/tests/paranoia.cpp) of PARANOIA,
a test suite written in Basic by William Kahan in 1983.
Paranoia is designed to discover obvious flaws in non-compliant floating point arithmetic and it is [still used today](https://dl.acm.org/doi/10.1145/1179622.1179682)!

```sh
git clone https://github.com/ssloy/tinyfloat.git &&
cd tinyfloat &&
cmake -Bbuild &&
cmake --build build -j &&
cd build &&
ctest . &&
./paranoia

```


