#include <stdio.h>

int main() {
    int age;
    float height;
    char initial;
    char name[20];

    printf("Enter your age: ");
    scanf("%d", &age);

    printf("Enter your height (in meters): ");
    scanf("%f", &height);

    printf("Enter your first initial: ");
    scanf(" %c", &initial); // Space before %c to consume any leftover newline

    printf("Enter your first name: ");
    scanf("%s", name);

    printf("You are %d years old, %.2f meters tall, your initial is %c, and "
           "your name is %s.\n",
           age, height, initial, name);

    return 0;
}
