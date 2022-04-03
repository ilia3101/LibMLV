# LibMLV

LibMLV is a library for reading MLV files (and writing, eventually). The API is inspired by the lua programming language's incredible API, so it's quite customisable - you can swap out the file reading backend and the memory allocator. The classes are also written to allocate memory in stack order, so you can make use of this property in custom allocators on embedded systems. I have a dream of implementing this library in magic lantern's mlv_play module some day, which I'm sure I'll never do 💀

### Notes

For windows, download "Build Tools for Visual Studio 2022" from https://visualstudio.microsoft.com/downloads/#other (scroll down) - I thjink?? Will check soon.