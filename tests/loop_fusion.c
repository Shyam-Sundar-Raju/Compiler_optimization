// Optimization technique: Loop Fusion
int main() {
    int i;
    int a = 0;
    int b = 0;
    for (i = 0; i < 10; i = i + 1) {
        a = a + i;
    }
    for (i = 0; i < 10; i = i + 1) {
        b = b + i * 2;
    }
    return a + b;
}
