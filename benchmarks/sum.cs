// sum.cs
using System;

class Program
{
    static void Main(string[] args)
    {
        // Example C# code
        int sum = 0;
        int[] array = new(100);
        for (int i = 0; i < array.Length; i++)
        {
            sum += array[i];
        }
        Console.WriteLine(sum);
    }
}
