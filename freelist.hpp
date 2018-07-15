// Copyright 2018 Katolikian Tihran
// created with inspiration from Paul Glinker's topic
// in "Game Programming Gems 4" book about preventing
// memory fragmentation

#ifndef FREELIST_HPP
#define FREELIST_HPP

#include <cassert>
#include <stdexcept>
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

// FreeList can prevent fragmentation, improve
// locality of reference, has a simple interface,
// is type-safe, reusable. It is NOT THREAD_SAFE YET.
//! TODO: thread safety

template <class Type>
class FreeList
{
public:
    // --------------------------
    // creates a FreeList which can handle "init_list_size"
    // objects of type "Type"
    explicit FreeList(const size_t init_list_size);

    ~FreeList();

    // ---------------------
    // returns pointer to the free segment in FreeList.
    // memory allocated on this pointer should be freed before this call,
    // because otherwise there is a risk it will be overrided
    Type *getFreePlace();
    
    // ---------------------
    // marks pointer as free. Do not manage memory,
    // operates only pointer.
    void markAsFree(Type * const ptr);

private:
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

    // ---------------------
    // marks all memory as free.
    // Required only for initialization
    void freeAll();
};

template <class Type>
FreeList <Type>::FreeList(const size_t init_list_size)
try : list_size(init_list_size),
      data(new char[list_size * sizeof(Type)]),
      free_segments(new char *[list_size])
    {
#ifdef _DEBUG
        std::cout << "Construction of FreeList (recieved size = " << init_list_size
                  << ")\nAllocated " << sizeof(Type) * list_size << " bytes for segments.\n"
                  << "Data starts from ptr: " << (void *)data << '\n';
#endif // _DEBUG
        freeAll();
    }
    catch (std::bad_alloc &) {
        // ----------------------
        // throw bad alloc exception to the user code.
        throw;
}

template <class Type>
FreeList <Type>::~FreeList()
{
#ifdef _DEBUG
    std::cout << "Destruction of FreeList.\n";
#endif // _DEBUG
    delete [] data;
    delete [] free_segments;
}

template <class Type>
Type *FreeList <Type>::getFreePlace()
{
        // ---------------------
        // check is there is at least one free place
        if (index_top == 0)
            throw std::runtime_error("FreeList stack overflow\n");

#ifdef _DEBUG
        std::cout << "Get request. Will return ptr: "
                  << (void *)free_segments[index_top - 1]
                  << '\n';
#endif // _DEBUG

        // --------------------
        // return pointer to the free segment
        return reinterpret_cast <Type *>
               (free_segments[--index_top]);
}

template <class Type>
void FreeList <Type>::markAsFree(Type * const ptr)
{
#ifdef _DEBUG
    std::cout << "Free request. Will free ptr: " << (void *)ptr << '\n';
#endif // _DEBUG
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
void FreeList <Type>::freeAll()
{
    size_t index = list_size - 1;

    for (index_top = 0; index_top < list_size; ++index_top) {
        free_segments[index_top] = &(data[index-- * sizeof(Type)]);
    }
}

#endif // FREELIST_HPP
