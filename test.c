#include<stdio.h>

int main()
{
    int arr[3];

    printf("%p %p %p\n", &arr[0], &arr[1], &arr[2]);
    printf("%p %p %p\n", arr, arr + 1, arr + 2);

    return 0;
}
