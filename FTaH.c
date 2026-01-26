#include <stdio.h>

int findCycle(int nums[], int numsSize) {
    if (numsSize <= 1) {
        return -1;
    }

    int tortoise = nums[0];
    int hare = nums[0];

    do {
        tortoise = nums[tortoise];
        hare = nums[nums[hare]];
        if (hare < 0 || hare >= numsSize) {
            return -1;
        }
    } while (tortoise != hare);

    tortoise = nums[0];
    while (tortoise != hare) {
        tortoise = nums[tortoise];
        hare = nums[hare];
    }

    return tortoise;
}

int main() {
    int arr[] = {1, 3, 4, 2, 2, 2, 2};
    int size = sizeof(arr) / sizeof(arr[0]);
    int result = findCycle(arr, size);

    if (result != -1) {
        printf("Cycle starts at value: %d\n", result);
    } else {
        printf("No cycle found\n");
    }

    return 0;
}
