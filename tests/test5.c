// Optimization technique: Dead Code Elimination
int main() {
    int x = 10;
    int y = x + 5;
    int unused = y * 100;
    return y;
}
