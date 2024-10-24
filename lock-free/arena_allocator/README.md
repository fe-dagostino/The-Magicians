# [The-Magicians](https://github.com/fe-dagostino/The-Magicians/blob/master/README.md): [Lock-Free](https://github.com/fe-dagostino/lock-free)

Last update: 05 August 2022

## lock_free::arena_allocator  &  core::arena_allocator

An arena_allocator implementation that keep both `allocate()` and `deallocate()` to a complexity of O(1).
 
This arena_allocator is useful when :
 - in a program there is an extensive use of "new" and "delete" for a well-defined "data type";
 - we want to avoid memory fragmentation, since the program is supposed to run for long time without interruptions;
 - performances in our program are important and we want to avoid waste of CPU cycles.

Let's see how it works and how it is possible to keep allocation and deallocation with a complexity of O(1).

In the following [Figure 1] there is representation for the *arena_allocator*; In this specific example we are considering `chuck_size = 5` since we want to illustrate the mechanism and 5 memory_slot are more than enough for this scope. 

![Figure 1](https://github.com/fe-dagostino/The-Magicians/blob/master/lock-free/arena_allocator/.resources/arena_allocator_initial.svg?raw=true)

As we can see in [Figure 1], there are different elements:
* **MEMORY CHUNK**: in order to avoid massive call to `new` and `delete` there is a single allocation, one for each `chunks`, where the amount of memory is enough to store `chunk_size` items of type `data_t` plus some extra memory required to manage the structure. In fact, the drawback to have better performances, at least with this implementation, is that we need a pointer for each single `memory_slot`, so 32 bits or 64 bits depending on the CPU architecture. Moreover, we need to keep other two pointers, respectively `first` and `last`, used for integrity checks as well as for deallocating the memory once `arena_allocator` will be destroyed.
* **MEMORY SLOT**: each `memory_slot` is divide in two areas that are respectively reserved for `user_data` information and a *pointer* to next free `memory_slot`, this last when the `memory_chunk` is initialized will be initialized with the address of next contiguous `memory_slot`, but after that, so during operation, this value can contain any of the available address for memory slots, in any of the allocated `memory_chunk`. 
* **next_free** : a *pointer* to `memory_slot` that always contain the address of next free memory_slot to be released with a call to allocate(). If there are no available `memory_slot` value will be `nullptr`. 

Now lets see a minimalist code for `allocate()` and `deallocate()`:
```cpp
  data_t* arena_allocator::allocate( ) 
  { 
    memory_slot* pCurrSlot = _next_free;
    if ( pCurrSlot == nullptr )       // There are no free slot available, this can occur in
      return nullptr;                 // if our arena_allocator have been limited to a maximum 
                                      // amount of memory_slot (size_limit template parameter)
                                      // or in case the system  run out-of-memory.

    
    // When we still have free memory_slot then the following 3 steps are done.
    
    _next_free = pCurrSlot->next();   // STEP 1: set _next_free to the next() in the chain.
                                      //         with reference to Figure 1, if _next_free was
                                      //         referring to 'Slot_1', then we move it to 
                                      //         Slot_1->next then means 'Slot_2'
    
    pCurrSlot->_next = nullptr;       // STEP 2: set next pointer to nullptr since this 
                                      //         memory_slot is "in use" and 'ownership'
                                      //         is released to the user.     
    
    return pCurrSlot->prt();          // STEP 3: release a pointer to data_t to the caller
  }
```

The above method `allocate()` basically performs:
* 1 x initial check
* 2 x assignment respectively at STEP 1 and STEP 2

```cpp
  void arena_allocator::deallocate( data_t* userdata ) noexcept
  {
    assert( userdata != nullptr );

    slot_pointer  pSlot = memory_slot::slot_from_user_data(userdata);

    userdata->~data_t();              // invoke data_t() destructor like a call to delete()

    pSlot->_next = _next_free;        // STEP 1: setting _next to _next_free, so following
                                      //         the status after allocate() above, we have
                                      //         pSlot = Slot_1, pSlot->_next = nullptr and
                                      //         and _next_free = Slot_2; then after this 
                                      //         assignment we will have:
                                      //           pSlot->next = Slot_2  

    _next_free = pSlot;               // STEP 2: _next_free = Slot_1, so in this specific
                                      //         use case we roll-back to the original status.

  }
```
The above method `deallocate()` basically performs:
* 1 x subtraction done by memory_slot::slot_from_user_data()
* 2 x assignment respectively at STEP 1 and STEP 2

In the above examples, there is no mention for `constructor` and `destructor` time, since this are `data_t` dependent and then in charge of the user in any case. Moreover, both constructor and destructor are executed out of any synchronization mechanism, so do not have impact on arena_allocator performances in a concurrent environment.

Please note that both the above methods are simplified, since there is no synchronization for the full implementation, please refer to [arena_allocator.h](https://github.com/fe-dagostino/lock-free/blob/master/include/arena_allocator.h) or to [core/arena_allocator.h](https://github.com/fe-dagostino/lock-free/blob/master/include/core/arena_allocator.h) where there are two different methods for both `allocate()` and `deallocate()`, one version is thread *safe* the other one is *unsafe*, then synchronization for *unsafe_* version is in charge to the caller. 

Let's do some `benckmarks` in different conditions. 

### Single-thread benckmarks

First results are obtained in without `multi-threads`, so without `context switch` and without `concurrency`, since we want to see the differet behaviours in different conditions. The following first table have been  obtained using [nanobench](https://nanobench.ankerl.com/) with the [bm_arena_allocator.cpp](https://github.com/fe-dagostino/lock-free/blob/master/benchmarks/bm_arena_allocator.cpp). Compiler `gcc 12.0.1 20220319` CPU `11th Gen Intel(R) Core(TM) i7-11800H @ 2.30GHz`.

|               ns/op |                op/s |    err% |          ins/op |          cyc/op |    IPC |         bra/op |   miss% |     total | benchmark
|--------------------:|--------------------:|--------:|----------------:|----------------:|-------:|---------------:|--------:|----------:|:----------
|               16.92 |       59,111,448.27 |    0.2% |          233.62 |           38.92 |  6.003 |          53.62 |    0.0% |      7.63 | `new and delete`
|               21.60 |       46,293,002.98 |    0.0% |           53.00 |           49.74 |  1.066 |           7.00 |    0.0% |      9.74 | `lock_free::arena_allocator`
|               20.77 |       48,149,902.91 |    0.1% |           46.00 |           47.81 |  0.962 |           4.00 |    0.0% |      9.41 | `lock_free::arena_allocator unsafe`
|               12.40 |       80,662,221.74 |    0.9% |          186.00 |           28.53 |  6.518 |          43.00 |    0.0% |      5.67 | `core::arena_allocator`
|                8.61 |      116,171,334.50 |    1.4% |           36.00 |           19.81 |  1.817 |           3.00 |    0.0% |      3.88 | `core::arena_allocator unsafe`

The table show the difference between the classic `new` and `delete` compared with `allocate()` and `deallocate()` in the *arena_allocator* in both modes *safe* and *unsafe* modes and both implementations.

In this specific `benchmark` we have only one thread allocating and deallocating memory, but when your program has many other calls to new and delete with different sizes, then running the program for hours will degrade `new` and `delete` performances due to fragmentation when the arena_allocator keep performances constant in time.

There is nothing magic since the arena allocator leverage the fact that it has all `memory_slot` with the same size, instead the `new` and `delete` should deal with generic requests and even with `tcache` optimization and/or `buddy` implementations the `new` operation will cost more than O(1) to search a new slot for the user.

Now that we saw what happens with a single thread, lets move in a `multi-threads` environment, in this new condition we will only use thread friend methods.

### Multi-threads benckmarks envitonment 

Just note that we are going to generate an environment with an High-Contended resource only for the reason we are interested to see the behaviour of our implementations in extreme conditions as well as the behaviour of `new` and `delete` operators that rely on `malloc` and on the system I'm running leverage `mmap`, but we will come back on this topic at the end. What is important to highlight is that extreme conditions like this shall be avoided in any application design.

* For the following `benchmarks` each `run` will execute 50 Millions allocation and 50 Millions deallocation; we want to keep total number of operations as constant in order better focus on time variation increasing the number of concurrent threads. So `first run` will do all operations with one thread, for the `second run` operations will be distributed on 2 theads and this means that each thread will do 25 Millions and 25 Millions deallocation and so on up to maximum number of threads.
* Number of thread will start from 1 and will be increased with step of 1 up to 16 threads, so for each implementation we come out with `16 runs`.
* Results are related to: `gcc 12.0.1 20220319` CPU `11th Gen Intel(R) Core(TM) i7-11800H @ 2.30GHz`, using a different compiler such as `MSVC` or `CLang` can bring different results, since they rely on different implementations with different solutions. In the following I will present only results generate with `GCC` compiler.
* Source code used for this specific benckmark [bm_mt_arena_allocator.cpp](https://github.com/fe-dagostino/lock-free/blob/master/benchmarks/bm_mt_arena_allocator.cpp)
* Results are also available [mt_results.ods](.resources/mt_results.ods)
* for each row we will compute the AVG and then this last will be used to compare the 3 results + one more results obtained using `std::mutex` instead of `core::mutex`. 

### Results for `new` and `delete` - System

|Threads|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|AVG|
|---:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|
|1|1.301||||||||||||||||1.301
|2|0.631|0.66|||||||||||||||0.646
|3|0.507|0.519|0.476||||||||||||||0.501
|4|0.373|0.443|0.422|0.37|||||||||||||0.402
|5|0.426|0.366|0.377|0.329|0.312||||||||||||0.362
|6|0.406|0.327|0.41|0.32|0.295|0.338|||||||||||0.349
|7|0.373|0.371|0.355|0.342|0.317|0.26|0.304||||||||||0.332
|8|0.328|0.325|0.347|0.329|0.246|0.298|0.246|0.283|||||||||0.300
|9|0.331|0.335|0.253|0.253|0.327|0.243|0.301|0.279|0.309||||||||0.290
|10|0.293|0.249|0.268|0.229|0.302|0.333|0.343|0.231|0.229|0.255|||||||0.281
|11|0.318|0.282|0.228|0.32|0.257|0.228|0.305|0.209|0.284|0.255|0.251||||||0.268
|12|0.319|0.312|0.252|0.276|0.313|0.274|0.251|0.274|0.195|0.244|0.266|0.241|||||0.284
|13|0.3|0.31|0.27|0.266|0.252|0.195|0.234|0.291|0.238|0.264|0.285|0.294|0.233||||0.265
|14|0.235|0.288|0.298|0.298|0.223|0.289|0.303|0.244|0.188|0.289|0.235|0.294|0.222|0.227|||0.272
|15|0.3|0.301|0.3|0.308|0.313|0.256|0.305|0.302|0.295|0.297|0.287|0.311|0.288|0.116|0.244||0.298
|16|0.301|0.248|0.291|0.29|0.241|0.285|0.293|0.284|0.278|0.297|0.282|0.289|0.274|0.278|0.157|0.146|0.279

### Results for `lock_free::arena_allocator` - Lock Free

|Threads|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|AVG|
|---:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|
|1|1.04||||||||||||||||1.040
|2|7.193|6.676|||||||||||||||6.935
|3|8.958|8.763|9.104||||||||||||||8.942
|4|9.297|9.401|8.86|9.366|||||||||||||9.231
|5|9.189|9.408|9.47|9.58|9.458||||||||||||9.421
|6|9.887|10.103|8.284|10.096|9.803|10.175|||||||||||9.725
|7|10.663|8.453|10.758|10.767|10.366|10.852|10.792||||||||||10.379
|8|11.301|10.912|11.295|8.893|10.996|10.07|11.353|11.106|||||||||10.741
|9|10.997|7.877|9.067|10.307|10.771|10.823|11.203|10.526|11.189||||||||10.196
|10|9.175|10.239|10.006|9.961|7.228|8.042|11.007|9.746|10.985|10.499|||||||9.426
|11|9.347|6.603|7.463|10.227|9.85|8.551|10.935|9.038|10.92|10.389|10.291||||||9.002
|12|10.657|7.708|10.731|8.599|9.859|11.076|8.301|11.099|6.006|10.773|9.167|10.738|||||9.754
|13|10.684|8.997|8.308|10.485|9.928|10.637|8.015|10.537|10.676|9.401|10.559|10.501|10.265||||9.699
|14|8.81|10.208|9.529|10.851|10.471|10.281|7.728|10.314|6.222|10.221|9.173|9.577|10.839|10.384|||9.774
|15|9.82|9.439|9.977|8.706|9.584|10.37|7.27|10.387|9.957|8.695|9.405|9.898|9.975|9.693|9.974||9.444
|16|10.002|9.253|9.297|9.923|7.991|10.497|10.063|10.491|8.783|10.028|9.92|9.135|9.513|8.575|8.045|10.082|9.690

### Results for `core::arena_allocator` - Spinlock

|Threads|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|AVG|
|---:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|------:|
|1|0.732||||||||||||||||0.732
|2|6.527|6.662|||||||||||||||6.595
|3|7.527|7.144|7.995||||||||||||||7.555
|4|11.509|12.235|13.447|13.269|||||||||||||12.615
|5|18.322|20.645|20.954|21.15|20.876||||||||||||20.389
|6|20.058|13.398|18.757|21.403|18.865|21.41|||||||||||18.982
|7|22.631|22.271|22.68|18.891|19.795|13.406|20.827||||||||||20.072
|8|25.28|22.443|19.029|25.518|22.126|21.79|19.746|25.482|||||||||22.677
|9|26.245|27.364|23.283|17.574|15.791|27.325|14.596|21.768|27.352||||||||21.743
|10|25.093|27.146|25.93|14.911|14.773|29.44|22.381|30.523|30.345|30.517|||||||23.775
|11|29.139|25.356|27.401|18.279|14.591|31.17|31.131|23.907|29.547|27.278|31.078||||||25.122
|12|34.612|32.115|13.793|19.552|15.229|35.082|23.196|31.419|31.696|34.031|31.894|35.099|||||25.625
|13|39.23|25.543|38.904|34.507|41.056|40.871|40.163|41.068|14.414|28.761|39.404|31.804|38.503||||37.668
|14|23|34.515|35.448|12.444|35.778|37.65|18.954|37.935|24.742|37.937|28.553|31.715|34.714|37.801|||29.466
|15|48.109|38.414|48.091|47.921|24.952|48.058|47.325|41.911|35.352|47.396|46.152|46.477|46.645|42.713|42.588||43.098
|16|49.105|44.869|39.921|51.058|42.998|50.617|49.154|51.268|51.296|49.352|43.876|51.499|51.5|49.531|35.247|47.484|47.374

From the above data is already clear what happened, but for a better view let's put all in a graph that include also a different implementation that leverage on `std::mutex`.
Note: if someone want to repeat the tests also with the `std::mutex`, the only change to be done in [core/arena_allocator.h](https://github.com/fe-dagostino/lock-free/blob/master/include/core/arena_allocator.h) is the following:
``` 
    ..
    ....
    //mutable mutex               _mtx_next;
    mutable std::mutex            _mtx_next;
    ..
    ...
```

### Comparison between all results

![All](https://github.com/fe-dagostino/The-Magicians/blob/master/lock-free/arena_allocator/.resources/compare_4.svg?raw=true)

This image, confirm what we have seen before in the first [benchmark](#single-thread-benckmarks) a single thread context, so all implementation show the same performances or better performances than `new` and `operator`, but this is valid only for the 1 thread, since as soon as we move to a multi-thread everything changes.
What is emerging from this first graph are the results obtained with std::mutex, that on this system leverage on pthread_mutex but I believe that also with other compilers and in other OS we can experiment the same type of degradation for the total time that reached up to 67 seconds to perform 100 Millions operations total, compared with the first result of 1.3 seconds required in a single thread context, with 16 threads we have about **67-1.3 = 65.7 seconds** lost from all threads on `waiting` to capture the `mutex`.   

Removing `std::mutex` results, we have a graph that is more intelligible and matches with the three tables presented above.

![All](https://github.com/fe-dagostino/The-Magicians/blob/master/lock-free/arena_allocator/.resources/compare_3.svg?raw=true)

Removing one element remaining information scaled up and it is more visible the results with one thread as well as the behaviour for all 3 allocation strategies. 
* **Spinlock**: we can observe that the trend is growing linearly, so increasing the number of threads the number also generate more collisions increase the time as well;
* **Lock-Free**: the results in a single thread environment are worst, since the sum of all atomic operations for a single call of `allocate()` or `deallocate()` are comparable to the time required to acquire a `mutex` or to create a "critical section" and to release it, but starting from 4 threads in parallel, we can observe that *lock-free* implementation is performing better than a spin-lock implementation, and the curious fact is that from this point forward performances seems to be not affected from the number of concurrent threads, in fact we have more or less the same results for each run. So as far we can see from these results, a Lock-Free implementation seems perfect for environments with high-contended resources. 
* **New & Delete**: bizarre results, right? with one single thread is one of the worst but after that it seems to improve the results and is going better and better increasing the number of threads. To understands what is going on, we have do dig a little bit in details in the malloc() implementation and we can find two reasons for those results.

First here a [link](https://sourceware.org/glibc/wiki/MallocInternals) for everyone interested to know more about *malloc* with the explanation on how it works internally. I really hope that this results will help to disrupt the misconceit that new() and delete() are slow, there is an huge amount of work behind this simple, but fundamental, functionality and a lot of research and improvements are still in progress; not perfect for sure, but improving release by release. 

*Please note that with other implementation results can be quite different, and yes in some implementation for `new` and `delete`.* 

Now let's see the two main reason to explain why we have this strange, sorry bizzarre, results on using "`new` and `delete`". 

In the following image there we have the comparison between `new` and `delete` and `std::mutex` that have respectively a time of **1.301s** and **1.344s**, quite closer, right? from this result, seems that `malloc()` internally is using a `mutex` and checking the implementation it is exactly like that.

![New & Delete comapre with mutex](https://github.com/fe-dagostino/The-Magicians/blob/master/lock-free/arena_allocator/.resources/compare_mutex.svg?raw=true)

At this point it is really important to remind, that the `benchmarks` have been executed taking the total number of operations as a constant, so this means that we have the following distribution increasing the number of threads.

![New & Delete comapre with mutex](https://github.com/fe-dagostino/The-Magicians/blob/master/lock-free/arena_allocator/.resources/mt_op_per_thread.png?raw=true)

And this simple reminder, can help to explain the following graph, in which increasing the number of threads the global time, here presented from the AVG, is going down in clear contrast with the trends observed for all other implementations.

![New & Delete](https://github.com/fe-dagostino/The-Magicians/blob/master/lock-free/arena_allocator/.resources/new_delete.svg?raw=true)

The above results, are real and they are possible only without contended resources, so from our observations seems that `new` & `delete` are acting like in single thread even there are multiple threads operating with those operators. Again the explanation for that is coming from malloc() implementation where in order to avoid or limit the number of resources shared between threads, to each thread is assigned an `arena`, this means that is like each thread have his own `new` and `delete` and depending on the number of total threads such arena isn't shared or better it is used only by one single thread, and if we complete this information with the above table, where increasing the number of threads we also reduce the number of operation per single thread then the graph make perfectly sense.

### Do we need an arena_allocator ?

Will not surprising if someone is asking them self, why do need an arena allocator if `new` and `delete` have such great performances. An arena_allocator is useful in many environments, but as we saw from all the results we must take care to use it in the right manner or we will waste a lot of CPU computational power, as well as energy, obtaining wretched performances. An arena allocator is a `building block` that can have many allocation strategies, helping to resolve issues like `memory fragmentation` and also to obtain a relevant improvement in performances. 

## Thanks for reading

Hope you enjoyed this article, and you find usful information in it.
