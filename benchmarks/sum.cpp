#include <iostream>
#include <vector>

int main() {
    std::vector<int> array = {1, 2, 3, 4, 5};
    int sum = 0;
    for (auto item : array) {
        sum += item;
    }
    std::cout << sum << std::endl;
}
