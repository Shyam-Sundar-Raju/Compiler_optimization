// Optimization technique: Branch Simplification
int main() {
    int x = 5;
    if (1) {
        x = x + 2;
    } else {
        x = x + 100;
    }
    return x;
}
