#include <iostream>
#include <vector>

int main() {
    std::vector<std::vector<int>> matrix_a(10, std::vector<int>(10));
    std::vector<std::vector<int>> matrix_b(10, std::vector<int>(10));
    std::vector<std::vector<int>> result(10, std::vector<int>(10, 0));

    // Initialize matrices
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            matrix_a[i][j] = i * 10 + j + 1;
            matrix_b[i][j] = i * 10 + j + 1;
        }
    }

    // Matrix multiplication
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 10; ++k) {
                result[i][j] += matrix_a[i][k] * matrix_b[k][j];
            }
        }
    }

    // Output result matrix
    for (auto &row : result) {
        for (auto &elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}
