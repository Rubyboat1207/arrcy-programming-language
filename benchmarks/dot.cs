using System;

class DotProductBenchmark
{
    static void Main()
    {
        int N = 100000;
        double[] arrayA = new double[N];
        double[] arrayB = new double[N];
        double sum = 0;

        // Initialize arrays with constants
        for (int i = 0; i < N; i++)
        {
            arrayA[i] = i + 1;
            arrayB[i] = N - i;
        }

        // Compute the dot product
        for (int i = 0; i < N; i++)
        {
            sum += arrayA[i] * arrayB[i];
        }

        // Print the result
        Console.WriteLine(sum);
    }
}
