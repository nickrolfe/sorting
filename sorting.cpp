#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <stdint.h>
#include <string>
#include <vector>

std::vector<uint64_t> generate_random_values(size_t num_elements) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> dist;
  std::vector<uint64_t> result;

  result.reserve(num_elements);
  for (size_t i = 0; i < num_elements; i++) {
    uint64_t value = (uint64_t)dist(gen);
    result.emplace_back(value);
  }
  return result;
}

std::vector<uint64_t> generate_sorted_values(size_t num_elements) {
  std::vector<uint64_t> result;

  result.reserve(num_elements);
  for (size_t i = 0; i < num_elements; i++) {
    result.emplace_back(i);
  }
  return result;
}

std::vector<uint64_t> generate_reversed_values(size_t num_elements) {
  std::vector<uint64_t> result;

  result.reserve(num_elements);
  for (size_t i = 0; i < num_elements; i++) {
    result.emplace_back(num_elements - i - 1);
  }
  return result;
}

std::vector<uint64_t> generate_organ_pipe_values(size_t num_elements) {
  std::vector<uint64_t> result;

  result.reserve(num_elements);
  for (size_t i = 0; i < num_elements / 2; i++) {
    result.emplace_back(i);
  }
  for (size_t i = 0; i < (num_elements - (num_elements / 2)); i++) {
    result.emplace_back(num_elements / 2 - i);
  }
  return result;
}

std::vector<uint64_t> generate_random_01_values(size_t num_elements) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> dist(0, 1);
  std::vector<uint64_t> result;

  result.reserve(num_elements);
  for (size_t i = 0; i < num_elements; i++) {
    uint64_t value = (uint64_t)dist(gen);
    result.emplace_back(value ? 1 : 0);
  }
  return result;
}

template <typename It> void my_insertion_sort(It begin, It end) {
  if (begin == end) {
    return;
  }
  for (It i = begin + 1; i != end; i++) {
    for (It j = i; j > begin && (*(j - 1)) > (*j); j--) {
      std::iter_swap(j, j - 1);
    }
  }
}

template <unsigned threshold, typename It>
static void my_qsort_helper(size_t level, It begin, It end) {
  while (level < 32 && end - begin > threshold) {
    auto pivot_val = *(begin + ((end - begin - 1) >> 1));
    It i = begin;
    It j = end-1;
    while (true) {
      while (*i < pivot_val) {
        i++;
      }
      while (*j > pivot_val) {
        j--;
      }
      if (i >= j) {
        break;
      }
      std::iter_swap(i, j);
      i++;
      j--;
    }
    level++;
    my_qsort_helper<threshold, It>(level, j + 1, end);
    end = j+1;
  }
  if (level >= 32) {
    std::make_heap(begin, end);
    std::sort_heap(begin, end);
  } else if constexpr (threshold > 1) {
    my_insertion_sort(begin, end);
  }
}

template <unsigned threshold, typename It> void my_qsort(It begin, It end) {
  my_qsort_helper<threshold, It>(0, begin, end);
}

template <unsigned power> void my_radix_sort(uint64_t *input, size_t size) {
  static_assert(64 % power == 0);
  constexpr size_t radix = 1llu << power;
  constexpr size_t mask = radix - 1;

  uint64_t *allocated_output = new uint64_t[size];
  uint64_t *output = allocated_output;
  uint64_t shift = 0;
  std::array<size_t, radix> counts;

  while (true) {
    counts.fill(0);

    size_t i;
    for (i = 0; i < size; i++) {
      counts[((input[i] >> shift) & mask)]++;
    }

    // Make count[i] contain position of the binary digit i in output
    size_t pos = counts[0];
    for (i = 1; i < radix; i++) {
      counts[i] += counts[i - 1];
    }

    i = size - 1;
    while (true) {
      size_t index = ((input[i] >> shift) & mask);
      size_t counts_fot_bit = counts[index]--;
      output[counts_fot_bit - 1] = input[i];
      if (i == 0) {
        break;
      }
      i--;
    }
    if (shift == (sizeof(uint64_t) * 8) - power) {
      break;
    }
    shift += power;
    std::swap(input, output);
  }

  delete[] allocated_output;
}

static int qsort_compare(const void *a, const void *b) {
  uint64_t x = *(const uint64_t *)a;
  uint64_t y = *(const uint64_t *)b;

  if (x < y) {
    return -1;
  }
  if (x > y) {
    return 1;
  }
  return 0;
}

typedef std::pair<const std::string,
                  std::function<void(std::vector<uint64_t> &)>>
    Sorter;
typedef std::pair<const std::string,
                  std::function<std::vector<uint64_t>(size_t)>>
    TestGenerator;

int main() {
  const std::vector<Sorter> sorters = {
      {"my_radix_sort (radix 2)",
       [](std::vector<uint64_t> &v) { my_radix_sort<1>(&v[0], v.size()); }},
      {"my_radix_sort (radix 4)",
       [](std::vector<uint64_t> &v) { my_radix_sort<2>(&v[0], v.size()); }},
      {"my_radix_sort (radix 16)",
       [](std::vector<uint64_t> &v) { my_radix_sort<4>(&v[0], v.size()); }},
      {"my_radix_sort (radix 256)",
       [](std::vector<uint64_t> &v) { my_radix_sort<8>(&v[0], v.size()); }},
      {"my_radix_sort (radix 65536)",
       [](std::vector<uint64_t> &v) { my_radix_sort<16>(&v[0], v.size()); }},
      {"my_qsort (threshold 1)",
       [](std::vector<uint64_t> &v) { my_qsort<1>(v.begin(), v.end()); }},
      {"my_qsort (threshold 16)",
       [](std::vector<uint64_t> &v) { my_qsort<16>(v.begin(), v.end()); }},
      {"my_qsort (threshold 32)",
       [](std::vector<uint64_t> &v) { my_qsort<32>(v.begin(), v.end()); }},
      {"my_qsort (threshold 64)",
       [](std::vector<uint64_t> &v) { my_qsort<64>(v.begin(), v.end()); }},
      {"std::sort",
       [](std::vector<uint64_t> &v) { std::sort(v.begin(), v.end()); }},
      {"std::stable_sort",
       [](std::vector<uint64_t> &v) { std::stable_sort(v.begin(), v.end()); }},
      {"std::sort_heap",
       [](std::vector<uint64_t> &v) {
         std::make_heap(v.begin(), v.end());
         std::sort_heap(v.begin(), v.end());
       }},
      {"C qsort",
       [](std::vector<uint64_t> &v) {
         qsort(&v[0], v.size(), sizeof(uint64_t), qsort_compare);
       }},
  };
  const std::vector<TestGenerator> test_generators = {
      {"Random uniform uint64 distribution", generate_random_values},
      {"Random [0, 1]", generate_random_01_values},
      {"Already sorted", generate_sorted_values},
      {"Reversed", generate_reversed_values},
      {"Organ pipe", generate_organ_pipe_values},
  };

#ifdef _DEBUG
  const size_t num_elements = 32;
#else
  const size_t num_elements = 1 << 15;
#endif

  for (auto &generator : test_generators) {
    std::cout << generator.first;
    const auto values = generator.second(num_elements);
    std::cout << " (" << values.size() << " elements)" << std::endl;

    for (auto &s : sorters) {
      auto values_copy = values;

      std::string spaces(36 - s.first.size(), ' ');
      std::cout << "    " << s.first << spaces;

      auto start = std::chrono::high_resolution_clock::now();
      s.second(values_copy);
      auto stop = std::chrono::high_resolution_clock::now();

      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(stop - start)
              .count();
      std::cout << duration << " us";

      if (!std::is_sorted(values_copy.begin(), values_copy.end())) {
        std::cout << " (FAILED)";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}
