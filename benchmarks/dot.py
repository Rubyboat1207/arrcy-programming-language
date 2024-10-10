N = 100000
array_a = [i + 1 for i in range(N)]
array_b = [N - i for i in range(N)]
sum = 0

# Compute the dot product
for i in range(N):
    sum += array_a[i] * array_b[i]

# Print the result
print(sum)
