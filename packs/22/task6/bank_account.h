#ifndef BANK_ACCOUNT_H
#define BANK_ACCOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bank_account bank_account_t;
typedef struct bank_account_vtable bank_account_vtable_t;

struct bank_account_vtable {
    void (*deposit)(bank_account_t *acc, double amount);
    int (*withdraw)(bank_account_t *acc, double amount);
    double (*get_balance)(bank_account_t *acc);
    const char *(*get_type)(bank_account_t *acc);
};

void deposit(bank_account_t *acc, double amount);
int withdraw(bank_account_t *acc, double amount);
double get_balance(bank_account_t *acc);
const char *get_type(bank_account_t *acc);
void destroy_account(bank_account_t *acc);

#ifdef __cplusplus
}
#endif

#endif
