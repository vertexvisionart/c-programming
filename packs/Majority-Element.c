void merge(int *arr, int left, int mid, int right) {
  int n1 = mid - left + 1;
  int n2 = right - mid;
  int L[n1];
  int R[n2];
  int k = left;
  int l = 0, r = 0;

  for (int i = 0; i < n1; i++) {
    L[i] = arr[left + i];
  }

  for (int i = 0; i < n2; i++) {
    R[i] = arr[mid + 1 + i];
  }

  while (l < n1 && r < n2) {
    if (L[l] <= R[r]) {
      arr[k] = L[l];
      l++;
    } else {
      arr[k] = R[r];
      r++;
    }
    k++;
  }

  while (l < n1) {
    arr[k] = L[l];
    l++;
    k++;
  }

  while (r < n2) {
    arr[k] = R[r];
    r++;
    k++;
  }
}

void mergesort(int *arr, int left, int right) {

  if (left < right) {
    int mid = left + (right - left) / 2;
    mergesort(arr, left, mid);
    mergesort(arr, mid + 1, right);
    merge(arr, left, mid, right);
  }
}

int majorityElement(int *nums, int numsSize) {
  mergesort(nums, 0, numsSize - 1);
  return nums[numsSize / 2];
}
