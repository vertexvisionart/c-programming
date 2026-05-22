#include "bank_account_p.h"

#include <stdlib.h>

void bank_account_init(bank_account_t *acc, int number, const bank_account_vtable_t *vtable)
{
    if (acc == NULL) {
        return;
    }
    acc->account_number = number;
    acc->balance = 0.0;
    acc->vtable = vtable;
}

void bank_account_base_deposit(bank_account_t *acc, double amount)
{
    if (acc == NULL || amount < 0.0) {
        return;
    }
    acc->balance += amount;
}

double bank_account_base_get_balance(bank_account_t *acc)
{
    return acc == NULL ? 0.0 : acc->balance;
}

void deposit(bank_account_t *acc, double amount)
{
    if (acc == NULL || acc->vtable == NULL || acc->vtable->deposit == NULL) {
        return;
    }
    acc->vtable->deposit(acc, amount);
}

int withdraw(bank_account_t *acc, double amount)
{
    if (acc == NULL || acc->vtable == NULL || acc->vtable->withdraw == NULL) {
        return 0;
    }
    return acc->vtable->withdraw(acc, amount);
}

double get_balance(bank_account_t *acc)
{
    if (acc == NULL || acc->vtable == NULL || acc->vtable->get_balance == NULL) {
        return 0.0;
    }
    return acc->vtable->get_balance(acc);
}

const char *get_type(bank_account_t *acc)
{
    if (acc == NULL || acc->vtable == NULL || acc->vtable->get_type == NULL) {
        return "Unknown";
    }
    return acc->vtable->get_type(acc);
}

void destroy_account(bank_account_t *acc)
{
    free(acc);
}
