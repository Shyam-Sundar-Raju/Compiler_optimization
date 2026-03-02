// Optimization technique: Loop Peeling
int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    int sum = 0;

    for (int i = 0; i < 5; i++) {
        if (i == 0) {
            sum += arr[i] * 2;
        } else {
            sum += arr[i];
        }
    }

    return sum;
}
