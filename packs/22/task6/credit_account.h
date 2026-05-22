#ifndef CREDIT_ACCOUNT_H
#define CREDIT_ACCOUNT_H

#include "bank_account.h"

#ifdef __cplusplus
extern "C" {
#endif

bank_account_t *create_credit_account(int number, double credit_limit);

#ifdef __cplusplus
}
#endif

#endif
