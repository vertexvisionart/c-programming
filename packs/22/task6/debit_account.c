#include "debit_account_p.h"

#include <stdlib.h>

static void debit_deposit(bank_account_t *acc, double amount)
{
    bank_account_base_deposit(acc, amount);
}

static int debit_withdraw(bank_account_t *acc, double amount)
{
    if (acc == NULL || amount < 0.0 || amount > acc->balance) {
        return 0;
    }
    acc->balance -= amount;
    return 1;
}

static double debit_get_balance(bank_account_t *acc)
{
    return bank_account_base_get_balance(acc);
}

static const char *debit_get_type(bank_account_t *acc)
{
    (void) acc;
    return "Debit";
}

static const bank_account_vtable_t debit_vtable = {
    debit_deposit,
    debit_withdraw,
    debit_get_balance,
    debit_get_type,
};

bank_account_t *create_debit_account(int number)
{
    debit_account_t *acc = (debit_account_t *) malloc(sizeof(*acc));
    if (acc == NULL) {
        return NULL;
    }
    bank_account_init(&acc->base, number, &debit_vtable);
    return &acc->base;
}
