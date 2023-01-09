#include<iostream>
#include<thread_pool/pool.hpp>
#include<ctime>
#include<cstdlib>
#include <unistd.h>
#include<tuple>

int main(void){
    srand(time(0));
    std::mutex cout_gaurd;

    std::cout << "Main Thread : " << std::this_thread::get_id() << "\n";

    my_pool::thread_pool tp;

    int temp_a = 9;
    for(auto i=0; i<10; ++i){

        // In testing passing anything will work
        auto my_func = [&,i=i](int my_temp){
            {
                std::unique_lock<std::mutex> gaurd(cout_gaurd);
                std::cout << "doing work " << i << "..." << "my_temp is: " << my_temp << '\n';
                sleep(5);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));
        };

        auto work_item = std::make_tuple(my_func,temp_a);
        tp.do_task(work_item);
    }

    return 0;
}

