matrix_a = [[i * 10 + j + 1 for j in range(10)] for i in range(10)]
matrix_b = [[i * 10 + j + 1 for j in range(10)] for i in range(10)]
result = [[0 for _ in range(10)] for _ in range(10)]

# Matrix multiplication
for i in range(10):
    for j in range(10):
        for k in range(10):
            result[i][j] += matrix_a[i][k] * matrix_b[k][j]

# Print result
for row in result:
    print(row)
