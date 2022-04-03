# LibMLV

LibMLV is a library for reading MLV files (and writing, eventually). The API is inspired by the lua programming language's incredible API, so it's quite customisable - you can swap out the file reading backend and the memory allocator. The main API is libmlv.h, and like lua, convenient functions that rely on the standard library/file IO are in libmlvaux.h, this clean separation again allows libmlv to be used on systems without traditional standard libraries (I have a dream of implementing this library in magic lantern's mlv_play module some day, which I'm sure I'll never do 💀).

The classes are also written to allocate memory in stack order, so you can make use of this property in custom allocators on embedded systems.

### Notes

For windows, download "Build Tools for Visual Studio 2022" (or whatever the latest year is) from https://visualstudio.microsoft.com/downloads/#other (scroll down)

When installing, check "Desktop development with C++", then click install:

![Screenshot 2022-04-03 at 17 12 52](https://user-images.githubusercontent.com/23642861/161437468-8db69298-0c37-4916-930e-64728a264471.png)

Use this command prompt for compiling:

![Screenshot 2022-04-03 at 17 30 16](https://user-images.githubusercontent.com/23642861/161438095-4f790d38-88e2-43c5-b9e6-27af08fe74d4.png)

On windows:
- `make` -> `NMAKE`
- `gcc`/`clang` -> `CL`
