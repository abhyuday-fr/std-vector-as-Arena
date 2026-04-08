#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <numeric>
#include <iomanip>

// Use a trick to prevent compiler from optimizing away code.
// This is more professional than just global sum.
#ifdef _MSC_VER
#include <intrin.h>
#pragma optimize("", off)
inline void do_not_optimize(void* p) { (void)p; }
#pragma optimize("", on)
#else
inline void do_not_optimize(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}
#endif

constexpr int N = 10'000'000;

struct Vector3 {
    int x{}, y{}, z{};
    Vector3(int x, int y, int z) : x(x), y(y), z(z) {}
};

long long global_sum = 0;

void benchmark_arena() {
    std::vector<Vector3> Arena;
    Arena.reserve(N);

    long long local_sum = 0;
    for (int i = 0; i < N; i++) {
        Arena.emplace_back(1, 2, 3);
        Arena[i].x += i;
        local_sum += Arena[i].x;
    }
    global_sum += local_sum;
    do_not_optimize(&global_sum);
}

void benchmark_new() {
    std::vector<Vector3*> ptrs;
    ptrs.reserve(N);

    long long local_sum = 0;
    for (int i = 0; i < N; i++) {
        Vector3* v = new Vector3{1, 2, 3};
        v->x += i;
        local_sum += v->x;
        ptrs.push_back(v);
    }

    for (auto p : ptrs) {
        delete p;
    }
    global_sum += local_sum;
    do_not_optimize(&global_sum);
}

void benchmark_vector_no_reserve() {
    std::vector<Vector3> vec;
    // No reserve, testing reallocation overhead

    long long local_sum = 0;
    for (int i = 0; i < N; i++) {
        vec.emplace_back(1, 2, 3);
        vec[i].x += i;
        local_sum += vec[i].x;
    }
    global_sum += local_sum;
    do_not_optimize(&global_sum);
}

template <typename Func>
double measure_time(Func func, int iterations = 5) {
    // Warmup
    func();

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count() / iterations;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <arena|new|no_reserve|all>\n";
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "arena") {
        benchmark_arena();
    } else if (mode == "new") {
        benchmark_new();
    } else if (mode == "no_reserve") {
        benchmark_vector_no_reserve();
    } else if (mode == "all") {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "--- Benchmarking (Average of 5 runs after warmup) ---\n";
        
        double t_arena = measure_time(benchmark_arena);
        std::cout << "Arena (vector reserve):      " << std::setw(10) << t_arena << " ms\n";

        double t_no_reserve = measure_time(benchmark_vector_no_reserve);
        std::cout << "Vector (no reserve):         " << std::setw(10) << t_no_reserve << " ms\n";

        double t_new = measure_time(benchmark_new);
        std::cout << "Individual new/delete:       " << std::setw(10) << t_new << " ms\n";
    } else {
        std::cerr << "Invalid mode: " << mode << "\n";
        return 1;
    }

    // Printing sum to ensure global_sum is used
    if (mode != "all") {
        std::cout << "Sum: " << global_sum << '\n';
    }

    return 0;
}
