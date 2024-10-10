using System;

class MatrixMultiplication
{
    static void Main()
    {
        // Initialize 10x10 matrices
        int[,] matrixA = new int[10, 10];
        int[,] matrixB = new int[10, 10];
        int[,] result = new int[10, 10];

        // Fill the matrices with values (1 to 100)
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                matrixA[i, j] = i * 10 + j + 1;
                matrixB[i, j] = i * 10 + j + 1;
            }
        }

        // Perform matrix multiplication
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                result[i, j] = 0;
                for (int k = 0; k < 10; k++)
                {
                    result[i, j] += matrixA[i, k] * matrixB[k, j];
                }
            }
        }

        // Print the result as a line-separated list
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                Console.WriteLine(result[i, j]);
            }
        }
    }
}
