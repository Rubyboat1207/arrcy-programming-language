const N = 100000;
let arrayA = new Array(N);
let arrayB = new Array(N);
let sum = 0;

// Initialize arrays with constants
for (let i = 0; i < N; i++) {
    arrayA[i] = i + 1;
    arrayB[i] = N - i;
}

// Compute the dot product
for (let i = 0; i < N; i++) {
    sum += arrayA[i] * arrayB[i];
}

// Print the result
console.log(sum);
