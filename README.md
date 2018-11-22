# glibc-windows
port glibc to win32 to work with MSVC.

There are many posix-like toolset porting on Windows. But, they are all designed to do a deep port. So they can be only work with GNU-toolset.
None of them can work with MS-VC.

I'd like to port a subset of glibc to windows working with MSVC.


## Goals:

* porting the io subset. fopen/fread/.../fmemopen/.. etc
* file/dir util funcs. getcwd/... etc.
* all utf-8 encoded. (hate msvc xxxA/xxxW funcs ^_^)

## Porting status
It's a large project, too many codes to mod.

I need help. Contact me!
