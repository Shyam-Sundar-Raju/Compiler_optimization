// Optimization technique: Loop Unrolling
int main() {
    int i;
    int sum = 0;
    for (i = 0; i < 8; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}
