/* Classic top-down merge sort on a fixed integer array. */
#include <stdio.h>

static void merge(int *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    int L[n1], R[n2];

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else              arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergesort(int *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergesort(arr, left, mid);
        mergesort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int main(void) {
    int arr[] = {32, 312, 33, 42, 234, 4234, 543, 1, 23, 4,
                 213, 321, 122, 33, 0, 0, 1, 32, 43};
    int size = sizeof(arr) / sizeof(arr[0]);
    mergesort(arr, 0, size - 1);
    for (int i = 0; i < size; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
