# [The-Magicians](../../README.md): templates are funny 

Last update: 14 August 2022

When talking about [templates](https://en.wikipedia.org/wiki/Template_(C++)), in most of the cases, the common opinion is that they are difficult to read, complex to write and debugging it means having a hard time. My personal opinion on this topic is that templates are the funniest part provided by C/C++ standard and continuously enriched with new functionalities. 

Here a pictorial definition for the template (source [ISO/C++ FAQ](https://isocpp.org/wiki/faq/templates)):

---
*A template is a cookie-cutter that specifies how to cut cookies that all look pretty much the same (although the cookies can be made of various kinds of dough, they’ll all have the same basic shape). In the same way, a class template is a cookie cutter for a description of how to build a family of classes that all look basically the same, and a function template describes how to build a family of similar looking functions.*

*Class templates are often used to build type safe containers (although this only scratches the surface for how they can be used).*

---

In this article we are not going to explain what templates are, instead we will focus on a practical example to show that sometimes usage of template can make to write less code, more readable and consequently more easily to debug as well as to maintain with a general saving of *precious* ***human time***. Surely don't take this as a general rule, in the following we will describe a technique, but it is up to you to decide when there is an advantage to using it.

So, let's start defining a context, then we will list the requirements, and then we will move into a possible implementation.

**Note**: for the rest of the article I'm using `g++` `v12.1` with `-std=c++20` on `x86_64 architecture`.

## Context

We are creating some basic containers such as a List and a Doubly-Linked-List, moreover we would like two different version of such containers, the first one let's say `STL` like (with a single context of execution), and the second one will be a `lock-free` implementation. 
In such context for all our implementation we will need an internal strucutre, a `node`, that will be used for the specific implementation.

## Requirements

Accordingly with our, above defined, context we should define 4 different types of nodes:
* a node with `single link` to *next* node
* a node with `atomic single link` to *next* node
* a node with `double link` to both *next* and *prev* node
* a node with `atomic double link` to both *next* and *prev* node

We can surely define 4 different structures, each one suitable for the specific implementation, but in order to demonstrate the versatility that we can obtain with templates we want to create a generic single structure suitable for all needs.

Let's try to list our requirements:
  1. one single structure
  2. an optional member _next (pointer to next node) can be selected at `compile-time`
  3. an optional member _prev (pointer to previous node) can be selected at `compile-time`
  4. for both _next and _prev, at `compile time`, can be possible to define them as atomic or raw

To implements all requirements, we will move *step-by-step* investigating the single *sub-problem* and then putting all together.

## Implementation 

So, let's cook/cut our cookie starting from the last requirement.

### Requirement #4 

What we want here is to have the possibility to choose the data type for both _next and _prev, this part is simplified by the fact that we already have all that we need from the standard library, in fact since C++11 we have [`std::conditional`](https://en.cppreference.com/w/cpp/types/conditional) and [`std::enable_if`](https://en.cppreference.com/w/cpp/types/enable_if) as part of [metaprogramming library](https://en.cppreference.com/w/cpp/meta) that is perfect for our scope, in this case we will use only the first one, so `std::conditional`.

The following alias is defined starting from C++14, let's see how it works.

```c++
template< bool B, class T, class F >
using conditional_t = typename conditional<B,T,F>::type;
```

The template offers three dirent paramenters, a **B**oolean and two generc types **T**rue and **F**alse. Accordingly with the value of B type T or F will be selected, the in our scope, supposing we have already defined a struct node_t and usage can be the following:

```c++

using pointer =  node_t*;  // creating this alias will simplify the rest once we will have all 
                           // template parameters for node_t, for now we keep it simple.

//                         |   B     | |        T          | |  F  |
typedef std::conditional_t<use_atomic, std::atomic<pointer>, pointer>   node_type;


node_type      _node;   // _node can be both a pointer, so a raw node_t* or std::atomic<node_t*> 
                        // depending on use_atomic value.

```

Now the following program should make sense :
```c++
#include <type_traits>
#include <iostream>
#include <atomic>

template<bool use_atomic>
struct node_t {
  static constexpr bool has_prev = true;

  node_t()
    : _node(nullptr)
  {}
  
  using pointer = node_t*;

  typedef std::conditional_t<use_atomic, std::atomic<pointer>, pointer>   node_type;

  node_type   _node;
};

int main()
{
  node_t<false>   _raw_node; 
  node_t<true>    _atomic_node;

  std::cout << typeid(_raw_node._node   ).name() << std::endl;
  std::cout << typeid(_atomic_node._node).name() << std::endl;

  return 0;
}

```

A possible output depending on the compiler can be the following:

```bash
P6node_tILb0EE
St6atomicIP6node_tILb1EEE
```
From the output it is clear that we got want we were expecting, so in the first case we have a pointer to node_t*, in the second case we can clear read the word `atomic` preceding the node_t*.

### Requirement #2 and #3

This two requirements are basically the same, since both are asking to have an optional memeber in our structure.
Also here we want to make use of template to find a solution.

The following code is a regular code: 

```c++
struct base {
};

struct derived : base {
  long  value;
};

```
Nothing special in there we declared an empty structure name `base` and extended this structure in `derived`. From there the basic idea to have two different version for `base`, one empty as for the code above and the other one containg a data memeber `long base_value`. 
The above can be easily obtained using template specialization.

```c++
#include <iostream>

// Base class a data member  base_value
template<bool empty_base>
struct base {
    long base_value;
};

// Template specialization for base then empty_base == true 
template<>
struct base<true> {
};

template<bool empty_base>
struct derived : base<empty_base> {
  long  value;
};

// Using example for our derived structure with both true and false in 
// order to check if everything is working as we were expecting
int main()
{
  derived<true>    _empty_base;
  derived<false>   _not_empty_base; 
  
  std::cout << sizeof(_empty_base    ) << std::endl; // return the size of a single long
  std::cout << sizeof(_not_empty_base) << std::endl; // return the size of two long

  return 0;
}
```

A possible output depending on the architecture of your machine can be the following:

```bash
8               // size of a single long on a x86_64 machine.
16              // size of a two long on a x86_64 machine.
```

Well done! So at this stage, we know how to solve requirements **#2**, **#3** and **#4** then all that we need is to put all together in order to create our generic node_t.

### Requirement #1

So let's try to summarise the structure that we need in the following template declaration.

```c++
/**
 * @brief forward declaration for node_t, intended for generic usage in queue stack, double linked list
 *        as well as in lock-free data structures.
 * 
 * @tparam value_type    there are not specific restriction, can be be anything form the appliation level.
 * @tparam add_prev      if true, node_t will have a data members _prev defined as a pointer to node_t
 *                       has_prev return true or false accordingly with this value.
 * @tparam add_next      if true, node_t will have a data members _nexr defined as a pointer to node_t
 *                       has_next return true or false accordingly with this value.
 * @tparam use_atomic    specify if _prev and _next should be atomic<node_t*> or simple node_t*.
 */
template<typename value_type,bool add_prev,bool add_next, bool use_atomic>
struct node_t;
```

It seems that we're going to cover all our requirements, and we can control everything from template parameters.

And here the full implementation for our generic node_t, all the code now should be clear since we just used the solutions we saw until this point.

```c++
#include <type_traits>
#include <iostream>
#include <atomic>

/**
 * @brief forward declaration for node_t, intended for generic usage in queue stack, double linked list
 *        as well as in lock-free data structures.
 * 
 * @tparam value_type    there are not specific restriction, can be be anything form the appliation level.
 * @tparam add_prev      if true, node_t will have a data members _prev defined as a pointer to node_t
 *                       has_prev return true or false accordingly with this value.
 * @tparam add_next      if true, node_t will have a data members _nexr defined as a pointer to node_t
 *                       has_next return true or false accordingly with this value.
 * @tparam use_atomic    specify if _prev and _next should be atomic<node_t*> or simple node_t*.
 */
template<typename value_type,bool add_prev,bool add_next, bool use_atomic>
struct node_t;

/**
 * @brief template structure that will be specialized with a data memeber by default.
 */
template<typename value_type,bool add_prev,bool add_next, bool use_atomic>
struct node_t_prev {
  static constexpr bool has_prev = true;

  constexpr inline node_t_prev()
    : _prev(nullptr)
  {}
  
  using pointer   = node_t<value_type,add_prev,add_next,use_atomic>*;
  using node_type = std::conditional_t<use_atomic, std::atomic<pointer>, pointer>;

  node_type   _prev;
};

/**
 * @brief specialization for add_prev == false.
 */
template <typename value_type,bool add_next,bool use_atomic>
struct node_t_prev<value_type,false,add_next,use_atomic> {
  static constexpr bool has_prev = false;
};

/**
 * @brief template structure that will be specialized with a data memeber by default.
 */
template<typename value_type,bool add_prev,bool add_next, bool use_atomic>
struct node_t_next {
  static constexpr bool has_next = true;

  constexpr inline node_t_next()
    : _next(nullptr)
  {}

  using pointer   = node_t<value_type,add_prev,add_next,use_atomic>*;
  using node_type = std::conditional_t<use_atomic, std::atomic<pointer>, pointer>;

  node_type   _next;
};

/**
 * @brief specialization for add_next == false.
 */
template <typename value_type,bool add_prev,bool use_atomic>
struct node_t_next<value_type,add_prev,false,use_atomic> {
  static constexpr bool has_next = false;    
};

/**
 * @brief check forward declaration above.
 *        node_t extends both node_t_prev<> and node_t_next<> 
 *        that accordingly with template parameters can be empty definitions
 *        or a pointer to prev and next.
 */
template<typename value_type, bool add_prev,bool add_next, bool use_atomic>
struct node_t : node_t_prev<value_type, add_prev, add_next, use_atomic>, 
                node_t_next<value_type, add_prev, add_next, use_atomic> 
{
  /**
   * @brief Default constructor.
   */
  constexpr inline node_t() noexcept
  { }

  /**
   * @brief Copy Constructor.
   */
  constexpr inline node_t( const value_type& value ) noexcept
    : _data(value)
  { }

  /**
   * @brief Constructor with move semantic.
   */
  constexpr inline node_t( value_type&& value ) noexcept
    : _data( std::move(value) )
  { }

  /**
   * @brief Destructor.
   */
  constexpr inline ~node_t() noexcept
  { }

  value_type    _data;
};

```

Let's give a try to the new structure and then we will highlight something more that is not clear from this specific example.

```c++
int main()
{
  node_t<long,false,false,false>   _node_only_data; 
  node_t<long,false,true ,false>   _node_data_and_next; 
  node_t<long,true ,false,false>   _node_data_and_prev; 
  node_t<long,true ,true ,false>   _node_data_next_prev;
  node_t<long,true ,true ,true >   _atomic_node_data_next_prev;

  std::cout << "\n _node_only_data      - size = " << sizeof(_node_only_data) <<  std::endl;
  std::cout << "  - has_prev = " << ((_node_only_data.has_prev)?"TRUE":"FALSE") << std::endl;
  std::cout << "  - has_next = " << ((_node_only_data.has_next)?"TRUE":"FALSE") << std::endl;
  
  std::cout << "\n _node_data_and_next  - size = " << sizeof(_node_data_and_next) <<  std::endl;
  std::cout << "  - has_prev = " << ((_node_data_and_next.has_prev)?"TRUE":"FALSE") << std::endl;
  std::cout << "  - has_next = " << ((_node_data_and_next.has_next)?"TRUE":"FALSE") << std::endl;

  std::cout << "\n _node_data_and_prev  - size = " << sizeof(_node_data_and_prev) <<  std::endl;
  std::cout << "  - has_prev = " << ((_node_data_and_prev.has_prev)?"TRUE":"FALSE") << std::endl;
  std::cout << "  - has_next = " << ((_node_data_and_prev.has_next)?"TRUE":"FALSE") << std::endl;

  std::cout << "\n _node_data_next_prev - size = " << sizeof(_node_data_next_prev) <<  std::endl;
  std::cout << "  - has_prev = " << ((_node_data_next_prev.has_prev)?"TRUE":"FALSE") << std::endl;
  std::cout << "  - has_next = " << ((_node_data_next_prev.has_next)?"TRUE":"FALSE") << std::endl;

  std::cout << "\n _atomic_node_data_next_prev - size = " << sizeof(_atomic_node_data_next_prev) <<  std::endl;
  std::cout << "  - has_prev = " << ((_atomic_node_data_next_prev.has_prev)?"TRUE":"FALSE") << std::endl;
  std::cout << "  - has_next = " << ((_atomic_node_data_next_prev.has_next)?"TRUE":"FALSE") << std::endl;

  return 0;
}
```
A possible output depending on the architecture of your machine can be the following:

```bash
_node_only_data             - size = 8
  - has_prev = FALSE
  - has_next = FALSE

_node_data_and_next         - size = 16
  - has_prev = FALSE
  - has_next = TRUE

_node_data_and_prev         - size = 16
  - has_prev = TRUE
  - has_next = FALSE

_node_data_next_prev        - size = 24
  - has_prev = TRUE
  - has_next = TRUE

_atomic_node_data_next_prev - size = 24
  - has_prev = TRUE
  - has_next = TRUE
```

All done! Just remind that I'm running on x86_64 machine and pointers are 8 bytes, so all results are coherent. 

### Some extra details

The structure node_t presented in this article is a really simple use case, but what we have seen can be applied in different contexts and one, for example, could be providing synchronized/raw data structures as part of standard library in order to allow the user to have at least basic data structures with mutexes or not. 

There is something more that is not obvious, so let's take our structure `node_t` to add the method `get_next()`.

An implementation for this get_next() at node_t level should take in account all possible conditions generated by template parameters.

Then supposing to add the following convenience alias and then the implementation get_next().

```c++
  using node_next_type = node_t_next<value_type,add_prev,add_next,use_atomic>;
  using pointer        = node_t<value_type,add_prev,add_next,use_atomic>*;
  
  /***/
  constexpr inline pointer get_next() const
  { 
    if constexpr ( node_next_type::has_next == true )  // also add_next can be used here.
    {
      if constexpr (use_atomic == true )
      {  return node_next_type::_next.load(); }

      if constexpr (use_atomic == false )
      {  return node_next_type::_next; }
    }
   
    return nullptr;
  }
```  
* Do we get a compiler error when we try to use the function get_next() ? 
            
      _next is not supposed to exists in all instances, so the above code should fail, right?  

The answer is **NO**, we don't get any error from the compiler, since we used `constexpr` in our if statements.
The above code compile with `GCC`, `CLang` and `MSVC`.

In the following assembly code, we can observe what happens at compile time. Just note that this is not optimized for the purpose to keep it readable, but when optimized this code will be drastically reduced or disappear, like as the first implementation that should be directly replaced with `nullptr`. 
 
```assembly
node_t<long, false, false, false>::get_next() const:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     eax, 0
        pop     rbp
        ret
node_t<long, false, true, false>::get_next() const:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     rax, QWORD PTR [rbp-8]
        mov     rax, QWORD PTR [rax]
        pop     rbp
        ret
node_t<long, true, false, false>::get_next() const:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     eax, 0
        pop     rbp
        ret
node_t<long, true, true, false>::get_next() const:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     rax, QWORD PTR [rbp-8]
        mov     rax, QWORD PTR [rax+8]
        pop     rbp
        ret
node_t<long, true, true, true>::get_next() const:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     QWORD PTR [rbp-8], rdi
        mov     rax, QWORD PTR [rbp-8]
        add     rax, 8
        mov     esi, 5
        mov     rdi, rax
        call    std::atomic<node_t<long, true, true, true>*>::load(std::memory_order) const
        nop
        leave
        ret
```
So basically we have 3 different implementation for get_next():
1. _next not present so `nullptr` will be returned ( first and third functions in the assembly )
2. _next as raw pointer ( second and fourth function in the assembly )
3. _next as atomic pointer ( fifth function in the assembly )

Leveraging usage of `constexpr` we obtained a method that can work on any specialization, methods can be also more complex for example can lock / unlock in the hypothesis that we have a data structure that potentially can work with a `mutex` as well as without.

## Conclusion

What we did in the above with `node_t`, is a good example when templates allow us to:
* write less code
* keep all implementation in one place, so avoid mistakes and bugs propagation
* specialize our code, compiling only what is used by the program
* implicitly test all methods and in most of the cases all possible paths
* register a general improvement in performances

But we have also some draw-back:
* learning curve for writing generic code that leverage template, meta-template and constexpr can be a bit flat, but once mastering basic concepts and having a good knowledge of available resources, then all becomes smoother
* in some cases, depending on both compiler and usage we do with templates the resulting link / executable can result bigger due to all specialization classes produced at compile time, so taking all under control especially for embedded systems is always a good idea
* *compile-time*, yes, clearly this increase following the complexity of our program. At the end, if we don't have specific requirement to build as fast as possible, this is an activity that is done once and generally pay with better performances for all the time the release remain active.

## Thanks for reading

I hope the content of this article was not boring and that you enjoyed reading it, and much better if the content can help you in your daily job within your projects.