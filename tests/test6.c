// Optimization technique: Unreachable Code Elimination
int main() {
    int x = 3;
    if (x > 0) {
        return 1;
        x = x + 100;
    }
    return 0;
}
