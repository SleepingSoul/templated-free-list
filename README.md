# FreeList documentation

I reccomend you to rewiew the [freelist.hpp](include/freelist.hpp) file for understanding of its interfase. There are many commentaries so it is
going to be easy. Moveover, this class is designed to have simple interface.

Few words about `FreeList` class:
It is created to prevent memory fragmentation and improve a locality of reference by preallocating a big block of memory for a particular number
of objects. So you should use it instead of simple allocating with `new` to get a great effect. It is recomended to use in game development, where
performance is very important.

Opportunities:
- If you want to use a thread safe variant of this library, you have to define "FL_THREAD_SAFETY" before including it.
