#include <iostream>
#include <vector>

int main()
{
    int N = 100000;
    std::vector<double> arrayA(N);
    std::vector<double> arrayB(N);
    double sum = 0;

    // Initialize arrays with constants
    for (int i = 0; i < N; ++i)
    {
        arrayA[i] = i + 1;
        arrayB[i] = N - i;
    }

    // Compute the dot product
    for (int i = 0; i < N; ++i)
    {
        sum += arrayA[i] * arrayB[i];
    }

    // Print the result
    std::cout << sum << std::endl;

    return 0;
}
