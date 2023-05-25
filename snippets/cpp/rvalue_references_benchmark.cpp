#include <iostream>
#include <string>
#include <cstddef>
#include <vector>
#include <chrono>


/*

without move:
   5463690795
    ctors_called: 1000
    copy_ctors_called: 1000
    move_ctors_called: 0

with move:
    2903161749
    ctors_called: 1000
    copy_ctors_called: 0
    move_ctors_called: 1000

with emplace_back:
    2902813439
    ctors_called: 1000
    copy_ctors_called: 0
    move_ctors_called: 0
*/



class ExpensiveToCreate
{
    public:
        ExpensiveToCreate()
            : ExpensiveToCreate(1)
        {

        }

        ExpensiveToCreate(std::size_t aSize) 
            : m_len(aSize)
        {
            m_data = new char[m_len];
            for(int i=0; i<m_len; i++) {
                m_data[i] = 0;
            }
            ctors_called++;
        }

        ExpensiveToCreate(const ExpensiveToCreate& aOther) 
            : m_len(aOther.m_len)
        {
            m_data = new char[m_len];
            for(int i=0; i<m_len; i++) {
                m_data[i] = aOther.m_data[i];
            }

            copy_ctors_called++;
        }

        /*
        ExpensiveToCreate(ExpensiveToCreate&& aOther) 
            : m_len(aOther.m_len)
        {
            m_data = aOther.m_data;
            aOther.m_data = 0;
            move_ctors_called++;
        }
        */

        virtual ~ExpensiveToCreate()
        {
            delete[] m_data;
        }

        static std::size_t ctors_called;
        static std::size_t copy_ctors_called;
        static std::size_t move_ctors_called;

    private:
        char* m_data;
        std::size_t m_len;

};

std::size_t ExpensiveToCreate::ctors_called = 0;
std::size_t ExpensiveToCreate::copy_ctors_called = 0;
std::size_t ExpensiveToCreate::move_ctors_called = 0;

int main()
{
    constexpr std::size_t rounds = 1000;

    std::vector<ExpensiveToCreate> bag;
    bag.reserve(rounds);

    auto start = std::chrono::system_clock::now();
    for(int iRound=0; iRound<rounds; iRound++) {
        
        if(false) {
            auto tmp = ExpensiveToCreate(1024*1024);
            bag.push_back(std::move(tmp));
        }
        else {
            bag.emplace_back(1024*1024);
        }
        
    }
    auto end = std::chrono::system_clock::now();

    auto elapsed = end - start;
    std::cout << elapsed.count() << '\n';


    std::cout<<"ctors_called: "<< ExpensiveToCreate::ctors_called<<std::endl;
    std::cout<<"copy_ctors_called: "<< ExpensiveToCreate::copy_ctors_called<<std::endl;
    std::cout<<"move_ctors_called: "<< ExpensiveToCreate::move_ctors_called<<std::endl;
}