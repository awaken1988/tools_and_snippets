//coroutine snippet
//	original From: https://godbolt.org/z/j4ah8K

#include <iostream>
#include <coroutine>


struct Awaiter {
 
    constexpr bool await_ready() const noexcept { return true; }
    
    void await_suspend(std::coroutine_handle<> h) { 
        std::cout << "*** Awaiter::await_suspend" << std::endl;
    }
    
    void await_resume() const noexcept {
        //std::cout << "Awaiter::await_resume" << std::endl;
    }
};

/*
 * HelloCoroutine:
 * The minimal machinery to use C++20 coroutines
 */
struct HelloCoroutine {
    /*
     * HelloPromise:
     * The minimal example coroutine promise
     */
    struct HelloPromise {
        HelloCoroutine get_return_object() {
            std::cout << "HelloCoroutine::get_return_object()" << std::endl;
            return std::coroutine_handle<HelloPromise>::from_promise(*this);
        }
        std::suspend_never initial_suspend() { 
            std::cout << "HelloCoroutine::initial_suspend()" << std::endl;
            return {}; 
        }
        
        std::suspend_always final_suspend() noexcept { 
            return {}; 
        }

        std::suspend_always yield_value(int value) noexcept {
            this->value = value;

            std::cout << "HelloCoroutine::yield_value(" << value << ")";

            return {};
        }



        void return_value(int value) { std::cout << "got " << value << "\n"; }
        void unhandled_exception() {}

        int value;
    };

    using promise_type = HelloPromise;
    HelloCoroutine(std::coroutine_handle<HelloPromise> h) : handle(h) {}

    std::coroutine_handle<HelloPromise> handle;
};

// Just a little helper for debugging
struct LifetimeInspector {
    LifetimeInspector(std::string s) : s(s) {
        std::cout << "Start: " << s << std::endl;
    }
    ~LifetimeInspector() {
        std::cout << "End: " << s << std::endl;
    }
    std::string s;
};

// Here is our coroutine
HelloCoroutine count_to_ten() {
    LifetimeInspector l("count_to_ten");

    for (int i = 0;; i++) {
        if (0 != (i % 2)) {
            co_await Awaiter{};
            continue;
        }
            
        co_yield i;
    }

    co_return 42;
}

// Usage of our coroutine
int main() {
    LifetimeInspector i("main");
    HelloCoroutine mycoro = count_to_ten();

    
    for (int i = 0; i < 5; i++) {
        std::cout << "calling resume" << std::endl;
        mycoro.handle.resume();

    }


    std::cout << "destroying stack frame" << std::endl;
    mycoro.handle.destroy();
}