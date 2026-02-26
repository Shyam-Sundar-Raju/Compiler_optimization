// Optimization technique: Common Subexpression Elimination
int main() {
    int x = 5;
    int y = 9;
    int p = (x * y) + 1;
    int q = (x * y) - 1;
    return p + q;
}
