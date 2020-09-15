# Arglet: A Declarative Argument-Parsing Library
Arglet is a header-only argument-parsing library written for C++, and it's designed with the goal of allowing you to write parsers that are 
- Simple;
- Visual;
- And Composable.
This means that parsers should be easy to write; the visual structure of the parser should reflect it's syntax; and you can build bigger parsers out of smaller parsers.
## Hello Arglet: A simple example program
Let's look at a simple parser for a program that will either print hello or goodbye:
```cpp
// hello-arglet.cpp
#include <iostream>
#include <arglet/arglet.hpp>

int main(int argc, char const** argv) {
    using namespace arglet;
    auto hello_tag = []{};
    auto goodbye_tag = []{};

    auto parser = sequence{
        ignore_arg, // We can ignore argv[0]
        flag_set{
            flag{hello_tag, 'h', "--hello"},
            flag{goodbye_tag, 'g', "--goodbye"}}};

    parser.parse(argc, argv);

    if(parser[hello_tag]) {
        std::cout << "Hello!" << std::endl;
    }
    if(parser[goodbye_tag]) {
        std::cout << "Goodbye!" << std::endl;
    }
}
```
We'll see the following output if we test the program:
```bash
$ g++ hello-arglet.cpp -o hello
$ ./hello --hello
Hello!
$ ./hello --goodbye
Goodbye!
```
Note that we can also use the single-letter short-hand flags, and that single-letter flags can be combined so that `-h -g` is the same as `-hg`
```bash
$ ./hello -h -g
Hello!
Goodbye!
$ ./hello -hg
Hello!
Goodbye!
```
### Parsers used by Hello Arglet
We use the following parsers in this program:
- `sequence`, which takes a list of parsers as arguments, and parses each in sequence
- `ignore_arg`, which ignores an argument (in this case, argv[0], which we don't care about)
- `flag_set`, which takes a list of flag parsers as arguments and parses them independent of the order in which they're entered, and
- `flag`, which takes a tag, and a shorthand name or a longhand name (or both). `flag` contains a boolean flag, which it sets to true if the parser encounters the flag.
### Getting information out of the parser
We can get information about what was parsed by calling `parser.operator[]` with the corresponding tag. 

In our example, the tags are `hello_tag` and `goodbye_tag`.  So `parser[hello_tag]` is true if the parser read `-h` or `--hello`, and `parser[goodbye_tag]` is true if the parser read `-g` or `--goodbye`.

Arglet uses this approach because it allows `operator[]` to return different types of values for different kinds of arguments. For example, a `value_flag` parses a flag followed by another argument that provides a value, and as a result `operator[]` needs to return the object parsed by the `value_flag`. 
