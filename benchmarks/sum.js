let array = Array.from({length: 100}, (_, i) => i + 1);
let sum = 0;
array.forEach((item) => {
    sum += item;
});
console.log(sum);
