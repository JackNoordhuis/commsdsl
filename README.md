# Overview
This project is a member of [CommsChampion Ecosystem](https://commschamp.github.io).
It provides multiple code generators to produce C++11 binary protocol
definition code as well as other satellite components that can be used to (fuzz) test
the protocol definition, visualize the message contents, debug the protocol messages exchange,
as well as create bindings (glue code) for other high level programming languages.

# What's Inside
- **commsdsl2comms** - A code generator, that produces C++11 code for binary
protocol definition out of [CommsDSL](https://github.com/commschamp/CommsDSL-Specification) 
schema files. The generated outcome is actually a CMake project that can be used to
properly install protocol definition headers as well as relevant cmake configuration files. 
For details on how to use the tool, please read the 
[commsdsl2comms Manual](doc/Manual_commsdsl2comms.md) 
documentation page. For details on the generated CMake project please read the
[Generated CMake Project Walkthrough](doc/GeneratedProjectWalkthrough.md)
documentation page.

- **commsdsl2test** - A code generator that produces C++11 code for fuzz
testing of the protocol definition produced by the **commsdsl2comms**.
Please read [Testing Generated Protocol Code](doc/TestingGeneratedProtocolCode.md) for
details
- **commsdsl2tools_qt** - A code generator, that produces the protocol
definition plugin code for [CommmsChampion Tools](https://github.com/commschamp/cc_tools_qt),
which can be used to visullize message contents as well as debug / observe exchange
of the messages between different systems.
- **commsdsl2swig** - A code generator that produces [SWIG](https://www.swig.org) interface 
file(s) for the protocol definition produced by the **commsdsl2comms**.
It allows generation of the bindings (glue code) to other high level 
programming languages using external [swig](https://www.swig.org) utility.
- **libcommsdsl** - A C++ library for parsing of 
[CommsDSL](https://github.com/commschamp/CommsDSL-Specification) schema files.
It can be used to implement independent code generators, which can produce
protocol definition code in other programming languages, bindings to the C++
classes generated by the **commsdsl2comms**, extra testing, etc... 
NOTE, that at this moment, the library is not documented. Please
[get in touch](#contact-information) in case you need it. I'll let you know
when it's going to be ready.

# License
The code of this project (libraries and tools it contains)
is licensed under [Apache v2.0](https://www.apache.org/licenses/LICENSE-2.0) license.

The generated code has no license, the vendor is free to
pick any as long as it's compatible with the
[license](https://commschamp.github.io/licenses/) of the
relevant [CommsChampion Ecosystem](https://commschamp.github.io) project.

# Tutorial
The [cc_tutorial](https://github.com/commschamp/cc_tutorial/) project contains a 
tutorial on how to use 
[CommsDSL](https://commschamp.github.io/commsdsl_spec/) to define binary communication protocol,
**commsdsl2comms** to generate code, and 
[COMMS Library](https://github.com/commschamp/comms) to customize and 
integrate the protocol definition with the business logic of the application.

# How to Build
Detailed instructions on how to build and install all the components can be
found in [doc/BUILD.md](doc/BUILD.md) file.

# Other Documentation
Please check the [doc](doc) folder for the available additional documentation.

# Versioning
This project will use [Semantic Versioning](https://semver.org/), where
**MAJOR** number will be equal to the latest **DSL** version 
(The first number of [CommsDSL](https://github.com/commschamp/CommsDSL-Specification)
version) it supports. The **MINOR** number will indicate various improvements
in the code of this repository, and **PATCH** number will indicate various bug fixes.

# Supported Compilers
This project (the code generator and [CommsDSL](https://github.com/commschamp/CommsDSL-Specification) 
parsing library) is implemented using C++17 programming language. As the result,
the supported compilers are:
- **GCC**: >=8
- **Clang**: >=7
- **MSVC**: >= 2017

The **generated** projects however contain C++11 valid code and supports a bit earlier
versions of the compilers:
- **GCC**: >=4.8
- **Clang**: >=3.8
- **MSVC**: >= 2015

# Branching Model
This repository will follow the 
[Successful Git Branching Model](http://nvie.com/posts/a-successful-git-branching-model/).

The **master** branch will always point to the latest release, the
development is performed on **develop** branch. As the result it is safe
to just clone the sources of this repository and use it without
any extra manipulations of looking for the latest stable version among the tags and
checking it out.

# Contact Information
For bug reports, feature requests, or any other question you may open an issue
here in **github** or e-mail me directly to: **arobenko@gmail.com**. I usually
respond within 24 hours.

