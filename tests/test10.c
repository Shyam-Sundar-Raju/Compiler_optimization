// Optimization technique: Induction Variable Elimination
int main() {
    int i;
    int j = 0;
    int sum = 0;
    for (i = 0; i < 10; i = i + 1) {
        j = i * 4;
        sum = sum + j;
    }
    return sum;
}
