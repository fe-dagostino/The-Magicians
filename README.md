# The-Magicians

Last update: July 2022

Who are The Magicians? or better, ***what*** are they? don't worry you are not in the wrong place and this repository in not intended to present you any extraordinary magical effect, in fact in this repository is focused on C/C++ and "The Magicians" is referred to all C/C++ compilers (g++, clang, msvc ...) since many developers think at the compiler has magical power to understand what he wants to do and then to optimize resulting assembly code in the best way. 

I'm sincerely sorry to bring bad news, but the compiler do not have any magic power! We must have a lot of reverence for all software engineers that every day are working on compilers to make them better and to cover new possible optimization, but even if they are extremely clever and capable, they cannot cover the context or the logic of all possible applications, so using the latest compiler with the latest standard do not avoid having bad programs or not well optimized assembly code.

Despite the title of this repository, it is intended as a collection of articles that face with specific topics, moving in depth on each one of them, explaining how it works and presenting objective results. All the topics will be covered avoiding to use specific jargon, instead a simple language will be preferred in order to cover all possible audience even without programming knowledge.

Different articles will be hard to be accepted since in some cases there are years of misinformation to be fought, but hoping in an *open-minded* approach from readers, all together we can avoid a lot of waste in terms of CPU cycles, with the benefits to have performing programs, less power consumption and as direct consequence of this last less impact on the environment since a little contribution can make the difference. 

In different articles we will talk about costs, taking in account all forms of costs and not only the [**Big-O notation**](https://en.wikipedia.org/wiki/Big_O_notation) for time and space complexity. More in details, depending on the specific content for the article we will consider:
- ***Time Complexity*** 
- ***Space Complexity***
- ***Compile Time***
- ***Execution time***: that in some cases reflect the time complexity, but this is not always true. So Time Complexity and Execution Time can really differs from each other.
- ***Generated code***
- ***Human Cost***

And as it happens in the real life, the best solution most of the time is to find out a compromise between all the five costs listed above.


Anyone that would like contributing to "The Magicians" will be more than welcome. 


Articles:

- [copy-elision](copy-elision/README.md)  --- *July 2022*
- [lock-free](lock-free/arena_allocator/README.md) --- *August 2022*
- [generic programmig](templates/generic-programming/README.md) --- *August 2022*
- smart pointers? --- wip


[comment]: <> (@todo compete complexity section)