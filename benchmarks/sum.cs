// sum.cs
using System;

class Program
{
    static void Main(string[] args)
    {
        // Example C# code
        int sum = 0;
        int[] array = { 1, 2, 3, 4, 5 };
        for (int i = 0; i < array.Length; i++)
        {
            sum += array[i];
        }
        Console.WriteLine(sum);
    }
}
