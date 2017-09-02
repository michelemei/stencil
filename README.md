# Stencil

**Stencil** is a utility for c ++ programmers who need to translate an array of bytes into structured data, and vice versa. It is common to have serialized data in binary format and deserialize them into classes or struct.
It is boring to have serialized data in binary format. To deserialize them in classes or structures, you must manually write all parsing of the binary stream. One way you can take is to use stencils.

## Features

Simple struct creation unsing `atom` and `pack` defines. E.g.:
```c++
struct point : michelemei::stencil<point> {
    using x = atom(double);
    using y = atom(double);
    pack(x, y);
    // ... your code
};
```
Access data quickly: the position of datas in the binary stream is computed at compile-time. E.g.:
```c++
std::cout << "X = " << pt.get<point::x>() << std::endl; // get the x coordinate of pt
ln.set<line::start, point::y>(1.2); // set the y coordinate starting point in ln
```

A simple and fast syntax to serialize and deserilize data:
```c++
// line is a class/struct inheriting from stencil
// my_line is ad instance of line

std::ofstream osf("line.bin", std::ios::out | std::ios::binary);
my_line.write(osf);          // Serialize
osf.close();

std::ifstream isf("my_obj.bin", std::ios::in | std::ios::binary);
line my_line = line(isf);    // Deserialize
isf.close();
```
