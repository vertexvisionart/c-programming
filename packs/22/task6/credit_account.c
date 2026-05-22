#include "credit_account_p.h"

#include <stdlib.h>

static credit_account_t *as_credit(bank_account_t *acc)
{
    return (credit_account_t *) acc;
}

static void credit_deposit(bank_account_t *acc, double amount)
{
    bank_account_base_deposit(acc, amount);
}

static int credit_withdraw(bank_account_t *acc, double amount)
{
    credit_account_t *credit = as_credit(acc);

    if (acc == NULL || amount < 0.0) {
        return 0;
    }
    if (credit->credit_limit < 0.0) {
        return 0;
    }
    if (acc->balance - amount < -credit->credit_limit) {
        return 0;
    }
    acc->balance -= amount;
    return 1;
}

static double credit_get_balance(bank_account_t *acc)
{
    return bank_account_base_get_balance(acc);
}

static const char *credit_get_type(bank_account_t *acc)
{
    (void) acc;
    return "Credit";
}

static const bank_account_vtable_t credit_vtable = {
    credit_deposit,
    credit_withdraw,
    credit_get_balance,
    credit_get_type,
};

bank_account_t *create_credit_account(int number, double credit_limit)
{
    credit_account_t *acc = (credit_account_t *) malloc(sizeof(*acc));
    if (acc == NULL) {
        return NULL;
    }
    bank_account_init(&acc->base, number, &credit_vtable);
    acc->credit_limit = credit_limit;
    return &acc->base;
}
