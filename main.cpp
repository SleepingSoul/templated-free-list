#include <iostream>
#include <vector>
#include "freelist.hpp"

class Mem
{
public:
    Mem(const char *str_to_print)
    : number(num++)
    {
        std::cout << "Num: " << number << ' ' << str_to_print;
    }
    ~Mem()
    {
        std::cout << "Destruction of num " << number << '\n';
    }
private:
    int number;
    static int num;
    char buf[200];
};

int Mem::num = 1;

int main()
{
    FreeList <Mem> mem_free_lst(8);
    
    auto p1 = mem_free_lst.getFreePlace(),
         p2 = mem_free_lst.getFreePlace();
    mem_free_lst.markAsFree(p1);
    mem_free_lst.getFreePlace();
}