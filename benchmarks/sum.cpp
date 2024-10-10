#include <iostream>
#include <vector>

int main() {
    std::vector<int> array(100);
    for (int i = 0; i < 100; ++i) array[i] = i + 1;
    int sum = 0;
    for (auto item : array) {
        sum += item;
    }
    std::cout << sum << std::endl;
}
