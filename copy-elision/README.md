# [The-Magicians](../README.md): copy-elision

Last update: 05 July 2022

To better understand what is the [*copy-elision*](https://en.cppreference.com/w/cpp/language/copy_elision) and all the benefits of such functionality we should go back in time, and more specifically before the introduction of [C++11](https://en.wikipedia.org/wiki/C++11) in which such optimization has been introduced for the first time together with the move semantic. The C++11 introduced significant changes in the standard but here we will focus on the move semantic and to the copy-elision functionality that where we will put all our attentions. In more recent standards, the *copy-elision* have been revisited in order to extend ranges of context where it can be applied from the compiler or when to use the move semantic instead. So for our discussion we have two specific moment in time, *before C++11* and *after C++11*.

Following, in our examples we will use an `std::vector<int>` that on a 64 bits system count a size of 24 bytes, it could be possible to use more complex and bigger data type, for example a custom structure or class to repeat all test with something bigger than 24 bytes, but what will be necessary is to have implemented at least `Default Constructor`, `Copy Constructor`, `Move Constructor`, `assignment operator` and `move assignment operator`, all of these are already present/implemented for all STL containers accordingly with used C++ standard.

## Before C++11

Let's start with c++98 and more specifically wiht something considered inefficient and clearly to be  avoid in order to preserve performances.

### Function 1 

The following function don't do too much, but it is enough for our scope. The function just create and return a `vector` of `integers` that contains `a` and `b`.


```cpp
std::vector<int> create_vector( int a, int b )
{
  std::vector<int> retValue;

  retValue.push_back(a);
  retValue.push_back(b);

  return retValue;
}
```

Now, we want to clarify why the above code for the [Function 1](#function-1) was badly performing and for that purpose we will define two use cases

### Function 1 - c++-98 - Use Case 1
```cpp

#include <iostream>
#include <vector>

int main()
{
  std::vector<int>  vec = create_vector( 10, 20 );

  std::cout << vec.size() << std::endl;

  return 0;
}

```

Provide source code is valid, so it possible to building it using an old compiler such as `x86-64 gcc 4.1.2` with the following flags `-std=c++98 -O3`. The program will be generated and will do the job.

But, let's dig a bit to see what happens when the code is executed.

1) the `main()` function will create on the stack an `std::vector<int>` named `vec` to receive output from `create_vector()` -- (24 bytes)
2) calling the function `create_vector()` we have:

    2.1) calling the function should create on the stack the return value and parameters for the function, so we have basically (24 bytes + 4 bytes + 4 bytes)
    
    2.2) once we are in the scope of `create_vector()`, so inside the function, then we have one more object that will be allocated on the stack (24 bytes) and then returned 

    2.3) when we exit from the scope of `create_vector()` so when execution reaches the return statement, then the value of `retValue` will be copied into the memory allocated for the return value at 2.1)

3) we are back to the `main()` function, and here finally we can initialize `vec` with the result of `create_vector()` and here the `copy constructur` will be invoked.

So in summary, we have `3 x allocations` for `std::vector<int>` and `2 x copy` that will be performed during the execution. That's a lot of extra work for returning a simple vector. So clearly there is room for few optimizations, but remember that right now we are working with c++98, so let's see what we can do, but just before that we inspect the second use case for [Function 1](#function-1). 

### Function 1 - c++-98 - Use Case 2

```cpp

#include <iostream>
#include <vector>

int main()
{
  std::vector<int>  vec;
  
  vec = create_vector( 10, 20 );

  std::cout << vec.size() << std::endl;

  return 0;
}

```

The difference between [Use Case 1](#function-1---c-98---use-case-1) and [Use Case2](#function-1---c-98---use-case-2) basically rely on allocating the vector with the default constructor and then using assign operator `=` to copy the result of `create_vector()` into `vec`, in terms of operation we just pay the bill for calling the `Default Constructor` for `std::vector<int>`, so not more than before but surely there are some extra operations.

Once we see what is supposed to be avoided, let's see a possible improvement to all that instances and copy operations.

### Function 2 

Following a little revision for the previous [Function 1](#function-1) in which we removed the return value by replacing it with `void` and then we introduced a new parameter that is a reference to an `std::vector<int> `.

```cpp
void create_vector( std::vector<int>& output, int a, int b )
{
  output.clear();

  output.push_back(a);
  output.push_back(b);
}
```

With this new function we expect that output vector will be provided when the function will be invoked, so we do not need to allocate internally a temporary variable and we don't need to return such object when we exit from the function. 

An using example for [Function 2](#function-2) is the following:

### Function 2 - c++-98 

```cpp

#include <iostream>
#include <vector>

int main()
{
  std::vector<int>  vec;
  
  create_vector( vec, 10, 20 );

  std::cout << vec.size() << std::endl;

  return 0;
}

```

In terms of operation we moved from `3 x instances` + `2 x copy` to
- `1 x instance`: we are creating one sigle object in the main function
- `0 x copy` : we removed all copy
- `1 x clear` : we have one extra call, we dind't see before that is clear() and is itended to guarantee that the vector is empty before to start pushing the two values.

Our intuition for sure will tell us that [Function 2](#function-2) have better performance than [Function 1](#function-1), and if so, please listen to your intuition since it is right and we will measure all of that.

## After C++11

After this short introduction :D, we finally arrived in the year 2011, and we all seen some radical changes in the C++ world, something good, something to be improved but surely moving forward and not static.

Before to proceed, I just have one question ... can you image how much code like [Function 1](#function-1) with related use cases have been written, and let me limit, from 1998 up to 2011 ? ... 

Ok, we don't care, since that what we defined `inefficient code` before C++11, now after the introduction of the new standard become the `canonical code` and this thanks to some changes that have been introduced with in all compilers that support the C++11. This also means that all code written before C++11 can now be recompiled with a new compiler and will automatically leverage from all new optimizations.

Those introductions and relevant for our discussion are:
- RVO & NRVO: Return Value Optimization and Named RVO. This kind of optimization are performed from the compiler under some circumstances and allow to avoid to copy objects between called and caller 
- Move semantic: that in our context, is the second option used from the compiler when RVO and NRVO cannot be applied.

In short the two bullet above are that's the [*copy-elision*](https://en.cppreference.com/w/cpp/language/copy_elision) so we finally arrived to the title of this article, but not to the end.

RVO and NRVO are basically intended to catch the same condition, that is when a local variable is returned to the caller the only difference consists in that NRVO is a variable declared in the scope of the function, instead in case of RVO this is a temporary object returned directly, so in our [Function 1](#function-1) we have a potential NRVO, instead if we were returning directly and instance such as `return std::vector<int>();` then in such case we have a potential RVO.

So what is going on with the [Use Case 1](#function-1---c-98---use-case-1) and [Use Case 2](#function-1---c-98---use-case-2) we analysed at the beginning of this article?

First of all the compiler will detect all function like our [Function 1](#function-1) in which a local variable is returned to the caller, and will mark all of them as RVO or NRVO.
Second, in all occurrence, so whether such [Function 1](#function-1) is invoked the compiler will try to optimize the code and to avoid extra allocation, so coming back to [Use Case 1](#function-1---c-98---use-case-1) we will have only one call object that will be created and this object will match with the memory allocated in the `main()` function. This is a consistent improvement if compared to where we start with `3 x instances` + `2 x copy`.
For the [Use Case 2](#function-1---c-98---use-case-2) we are not creating the object, instead we are using the assignment operator with an object that is already created, so in this case the compiler in order to avoid the copy of all elements in the local variable, and due to the fact that immediately after such object will be destroyed, will call automatically move all pointers and data from local object to the destination object, and then invalidate the local object. This means that in our specific case, where we have a vector, all its content will be moved to the destination object, so counters will be assigned, but allocated memory will just change ownership.


----- 
still work in progress ....



