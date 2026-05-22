#ifndef CREDIT_ACCOUNT_P_H
#define CREDIT_ACCOUNT_P_H

#include "bank_account_p.h"

typedef struct {
    bank_account_t base;
    double credit_limit;
} credit_account_t;

#endif
