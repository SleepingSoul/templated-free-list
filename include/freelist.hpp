// Copyright 2018 Katolikian Tihran
// created with inspiration from Paul Glinker's topic
// in "Game Programming Gems 4" book about preventing
// memory fragmentation

// define "FL_THREAD_SAFETY" to compile the thread safe
// variant of this library

#ifndef FREELIST_HPP
#define FREELIST_HPP

#include <cassert>
#include <stdexcept>

#ifdef FL_THREAD_SAFETY
#include <mutex>
#endif // FL_THREAD_SAFETY

// FreeList can prevent fragmentation, improve
// locality of reference, has a simple interface,
// is type safe, thread safe and reusable

template <class Type>
class FreeList
{
public:
    // --------------------------
    // creates a FreeList which can handle "init_list_size"
    // objects of type "Type"
    explicit FreeList(const size_t init_list_size);

    // --------------------------
    // constructor for pre-allocated data
    FreeList(Type * const init_data,
             Type ** const init_free_segments,
             const size_t init_list_size);

    // --------------------------
    // copy constructor is forbidden
    FreeList(const FreeList &) = delete;

    // --------------------------
    // assigment is forbidden for FreeList
    operator =(const FreeList &) = delete;

    // --------------------------
    // move constructor
    FreeList(FreeList &&rv);

    ~FreeList();

    // ---------------------
    // returns pointer to the free segment in FreeList.
    // memory allocated on this pointer should be freed before this call,
    // because otherwise there is a risk it will be overrided
    Type *getFreePlace();

    // ---------------------
    // acts as the previous one, but also created as object of type
    // "Type" in place and passes "args" in its constructor.
    template <class ...Args>
    Type *constructOnFreePlace(Args... args);
    
    // ---------------------
    // marks pointer as free. Do not manage memory,
    // operates only pointer.
    void markAsFree(Type * const ptr);

    // ---------------------
    // calls destructor for the object and then calls
    // "markAsFree" function
    void destructAndMarkAsFree(Type * const ptr);

    // ---------------------
    // return size in bytes allocated for
    // data
    size_t getPhysicalSize() const;

    // ---------------------
    // calculates the size will be allocated for data in
    // list of "size" elements
    static size_t calculatePhysicalSize(const size_t size);

private:
    // this value depends on constructor called
    // to create this instance of FreeList
    bool free_resources_on_destr;
    // size of the FreeList (number of objects which
    // can be stored here)
    const size_t list_size;
    // required for iterating the free_segments array
    // (stack)
    size_t index_top;
    // data for segments
    char *data;
    // pointers to free segments (stack)
    char **free_segments;

#ifdef FL_THREAD_SAFETY
    std::mutex fl_mutex;
#endif // FL_THREAD_SAFETY

    // ---------------------
    // marks all memory as free.
    // Required only for initialization
    void freeAll();
};

template <class Type>
FreeList <Type>::FreeList(const size_t init_list_size)
try : free_resources_on_destr(true),
      list_size(init_list_size),
      data(new char[list_size * sizeof(Type)]),
      free_segments(new char *[list_size])
{
    freeAll();
}
catch (std::bad_alloc &) {
    // ----------------------
    // throw bad alloc exception to the user code.
    throw;
}

template <class Type>
FreeList <Type>::FreeList(Type * const init_data,
                          Type ** const init_free_segments,
                          const size_t init_list_size)
: free_resources_on_destr(false),
  data(init_data),
  free_segments(init_free_segments),
  list_size(init_list_size)
{
    freeAll();
}

template <class Type>
FreeList <Type>::FreeList(FreeList &&rv)
: free_resources_on_destr(rv.free_resources_on_destr),
  list_size(rv.list_size),
  index_top(rv.index_top),
  data(rv.data),
  free_segments(rv.free_segments)
{
    // ------------------------
    // we dont want previous owner of resources to
    // free it, because there is a new owner
    rv.free_resources_on_destr = false;
}

template <class Type>
FreeList <Type>::~FreeList()
{
    if (free_resources_on_destr) {
        delete [] data;
        delete [] free_segments;
    }
}

template <class Type>
Type *FreeList <Type>::getFreePlace()
{
#ifdef FL_THREAD_SAFETY
    std::lock_guard <std::mutex> lg(fl_mutex);
#endif // FL_THREAD_SAFETY

    // ---------------------
    // check is there is at least one free place
    if (index_top == 0)
        throw std::runtime_error("FreeList stack overflow\n");

    // --------------------
    // return pointer to the free segment
    return reinterpret_cast <Type *>
            (free_segments[--index_top]);
}

template <class Type>
    template <class ...Args>
Type *FreeList <Type>::constructOnFreePlace(Args... args)
{
    return new (getFreePlace()) Type(args...);
}

template <class Type>
void FreeList <Type>::markAsFree(Type * const ptr)
{
#ifdef FL_THREAD_SAFETY
    std::lock_guard <std::mutex> lg(fl_mutex);
#endif // FL_THREAD_SAFETY

    // ----------------------
    // check if adress is correct
    assert(reinterpret_cast <char *>(ptr) >= data);
    assert(reinterpret_cast <char *>(ptr) <= data +
           (list_size - 1) * sizeof(Type));
    // ----------------------
    // check if there was at least one request
    // for pointer before
    assert(index_top < list_size);

    free_segments[index_top++] = reinterpret_cast <char *>
                                 (ptr);
}

template <class Type>
void FreeList <Type>::destructAndMarkAsFree(Type * const ptr)
{
#ifdef FL_THREAD_SAFETY
    std::lock_guard <std::mutex> lg(fl_mutex);
#endif // FL_THREAD_SAFETY

    markAsFree(ptr);
    ptr->~Type();
}

template <class Type>
size_t FreeList <Type>::getPhysicalSize() const
{
    return list_size * sizeof(Type);
}

template <class Type>
size_t FreeList <Type>::calculatePhysicalSize(const size_t size)
{
    return size * sizeof(Type);
}

template <class Type>
void FreeList <Type>::freeAll()
{
    size_t index = list_size - 1;

    for (index_top = 0; index_top < list_size; ++index_top) {
        free_segments[index_top] = &(data[index-- * sizeof(Type)]);
    }
}

#endif // FREELIST_HPP
