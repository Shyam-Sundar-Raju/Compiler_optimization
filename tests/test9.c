// Optimization technique: Loop Invariant Code Motion
int main() {
    int i;
    int sum = 0;
    int a = 4;
    int b = 5;
    for (i = 0; i < 10; i = i + 1) {
        sum = sum + (a * b) + i;
    }
    return sum;
}
