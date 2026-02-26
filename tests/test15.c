// Optimization technique: Redundant Assignment Elimination
int main() {
    int x = 0;
    x = 4;
    x = 9;
    int y = x + 1;
    return y;
}