#include <stdio.h>

// 1. The actual callback function
void my_callback(int result) {
    printf("Callback received result: %d\n", result);
}

// 2. A function that accepts a function pointer as an argument
void perform_calculation(int a, int b, void (*callback)(int)) {
    int sum = a + b;
    // 3. Execution of the callback
    callback(sum);
}

int main() {
    // Registering 'my_callback' by passing it as an argument
    perform_calculation(5, 10, my_callback);
    return 0;
}
