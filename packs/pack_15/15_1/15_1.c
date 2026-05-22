#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ERROR_CODE = 13 };

typedef enum { OP_ADD, OP_SUB, OP_MUL } OperationType;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "No parameters specified.");
    return ERROR_CODE;
  }

  long long res;
  long long first_value;
  long long second_value;
  long long module = 0;
  int idx = 1; // индекс чтения команды

  if (strcmp(argv[1], "-m") == 0) {
    module = atoll(argv[2]);
    idx = 3;
  } else if (argc >= 5 && strcmp(argv[4], "-m") == 0) {
    module = atoll(argv[5]);
  }

  char *op_str = argv[idx];
  first_value = atoll(argv[++idx]);
  second_value = atoll(argv[++idx]);

  OperationType op;
  if (strcmp(op_str, "add") == 0) {
    op = OP_ADD;
  } else if (strcmp(op_str, "sub") == 0) {
    op = OP_SUB;
  } else if (strcmp(op_str, "mul") == 0) {
    op = OP_MUL;
  }

  switch (op) {
  case OP_ADD:
    res = first_value + second_value;
    break;
  case OP_SUB:
    res = first_value - second_value;
    break;
  case OP_MUL:
    res = first_value * second_value;
    break;
  }

  if (module) {
    res = res % module;
    if (res < 0) {
      res = res + module;
    }
  }
  printf("%lld\n", res);

  return EXIT_SUCCESS;
}
