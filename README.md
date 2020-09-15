# Arglet: A Declarative Argument-Parsing Library
Arglet is a header-only argument-parsing library written for C++, and it's designed with the goal of allowing you to write parsers that are 
- Simple,
- Readable,
- and Composable

This means that parsers should be easy to write; the structure of the parser should reflect it's syntax; and you can build bigger parsers out of smaller parsers.
## Hello Arglet: A simple example program
Let's look at a simple parser for a program that will either print hello or goodbye:
```cpp
// hello-arglet.cpp
#include <iostream>
#include <arglet/arglet.hpp>

namespace tags {
    struct tag1 {} hello_tag;
    struct tag2 {} goodbye_tag;
}

int main(int argc, char const** argv) {
    using namespace arglet;

    auto parser = sequence{
        ignore_arg, // We can ignore argv[0]
        flag_set{
            flag{tags::hello_tag, 'h', "--hello"},
            flag{tags::goodbye_tag, 'g', "--goodbye"}
        }
    };

    parser.parse(argc, argv);

    if(parser[tags::hello_tag]) {
        std::cout << "Hello!" << std::endl;
    }
    if(parser[tags::goodbye_tag]) {
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
We can get information about what was parsed by calling `parser.operator[]` with the corresponding *tag*. Here, a *tag* is an instance of a stateless type that we use to pass information. There's an overload of `parser.operator[]` for each tag used in constructing `parser`, and the value `parser.operator[]` returns is taken from the flag (or other simple parser) used in constructing `parser`. 

In our example, the tags are `tags::hello_tag` and `tags::goodbye_tag`.  So `parser[tags::hello_tag]` is true if the parser read `-h` or `--hello`, and `parser[tags::goodbye_tag]` is true if the parser read `-g` or `--goodbye`.

Because each tag has it's own type, each overload for `operator[]` can return it's own type as well, meaning that you can parse complex information (like a list of files) all while keeping the implementation type-safe and statically typed:
```cpp
using path = std::filesystem::path;

auto parser = sequence {
    ignore_arg,
    list_of {
        tags::files, // Declared in namespace tags
        std::vector<path>{}
    }
};

parser.parse(argc, argv);

std::vector<path> files = parser[tags::files];
``` 
