let matrix_a = Array.from({length: 10}, (_, i) =>
    Array.from({length: 10}, (_, j) => i * 10 + j + 1)
);
let matrix_b = Array.from({length: 10}, (_, i) =>
    Array.from({length: 10}, (_, j) => i * 10 + j + 1)
);
let result = Array.from({length: 10}, () =>
    Array.from({length: 10}, () => 0)
);

// Matrix multiplication
for (let i = 0; i < 10; i++) {
    for (let j = 0; j < 10; j++) {
        for (let k = 0; k < 10; k++) {
            result[i][j] += matrix_a[i][k] * matrix_b[k][j];
        }
    }
}

// Print result
console.log(result);
