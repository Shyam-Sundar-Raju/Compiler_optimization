// Optimization technique: Function Cloning
int compute(int x) {
    if (x > 100) {
        return x * 2 + 1;
    }
    return x + 3;
}

int main() {
    int a = compute(5);
    int b = compute(200);
    return a + b;
}
