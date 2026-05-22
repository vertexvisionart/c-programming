#include <ctype.h>
#include <stdio.h>

#define MAXN 500005

double vals[MAXN];
int vtop = 0;

void vpush(double v) { vals[vtop++] = v; }
double vpop() { return vals[--vtop]; }

char ops[MAXN];
int otop = 0;

void opush(char c) { ops[otop++] = c; }
char opop() { return ops[--otop]; }
char otopval() { return ops[otop - 1]; }

int prec(char op) {
  if (op == 'u')
    return 3;
  if (op == '*' || op == '/')
    return 2;
  if (op == '+' || op == '-')
    return 1;
  return 0;
}

void applyOp() {
  char op = opop();
  if (op == 'u') {
    vpush(-vpop());
  } else {
    double b = vpop();
    double a = vpop();
    if (op == '+')
      vpush(a + b);
    else if (op == '-')
      vpush(a - b);
    else if (op == '*')
      vpush(a * b);
    else if (op == '/')
      vpush(a / b);
  }
}

char expr[MAXN];

int main() {
  fgets(expr, MAXN, stdin);

  int canUnary = 1;
  int i = 0;

  while (expr[i] != '\0' && expr[i] != '\n') {
    char c = expr[i];

    if (c == ' ') {
      i++;
      continue;
    }

    if (isdigit(c)) {
      double num = 0;
      while (isdigit(expr[i]))
        num = num * 10 + (expr[i++] - '0');
      vpush(num);
      canUnary = 0;
      continue;
    }

    if (c == '(') {
      opush('(');
      canUnary = 1;
      i++;
      continue;
    }

    if (c == ')') {
      while (otop > 0 && otopval() != '(')
        applyOp();
      if (otop > 0)
        opop();
      canUnary = 0;
      i++;
      continue;
    }

    if (c == '-' && canUnary) {
      while (otop > 0 && otopval() != '(' && prec(otopval()) > prec('u'))
        applyOp();
      opush('u');
      canUnary = 0;
      i++;
      continue;
    }

    while (otop > 0 && otopval() != '(' && prec(otopval()) >= prec(c))
      applyOp();
    opush(c);
    canUnary = 0;
    i++;
  }

  while (otop > 0)
    applyOp();

  printf("%.20f\n", vals[0]);
  return 0;
}
