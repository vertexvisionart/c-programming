#ifndef BANK_ACCOUNT_P_H
#define BANK_ACCOUNT_P_H

#include "bank_account.h"

struct bank_account {
    int account_number;
    double balance;
    const bank_account_vtable_t *vtable;
};

void bank_account_init(bank_account_t *acc, int number, const bank_account_vtable_t *vtable);
void bank_account_base_deposit(bank_account_t *acc, double amount);
double bank_account_base_get_balance(bank_account_t *acc);

#endif
