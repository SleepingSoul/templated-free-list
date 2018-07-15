#include <iostream>
#include <stack>

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
    std::stack <Mem *> ptrs;
    
    try {
        while (true) {
            ptrs.push(mem_free_lst.constructOnFreePlace("something\n"));
        }
    }
    catch (std::runtime_error &er)  {
        std::cout << "Exception! " << er.what();
    }

    while(!ptrs.empty()) {
        mem_free_lst.destructAndMarkAsFree(ptrs.top());
        ptrs.pop();
    }
}