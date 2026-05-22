#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct State {
    char *regs[256];
} State;

// Вспомогательная функция для дублирования строк
static char* my_strdup(const char* s) {
    if (!s) return NULL;
    char* copy = (char*)malloc(strlen(s) + 1);
    if (copy) strcpy(copy, s);
    return copy;
}

void concat_2(State *state, char *idx0, char *idx1) {
    int i0 = atoi(idx0);
    int i1 = atoi(idx1);

    // Получаем длины (учитываем, что регистр может быть NULL)
    int len0 = state->regs[i0] ? strlen(state->regs[i0]) : 0;
    int len1 = state->regs[i1] ? strlen(state->regs[i1]) : 0;

    // Выделяем новую память ПЕРЕД удалением старой!
    // Это защищает нас в случае, когда i0 == i1
    char *res = (char*)malloc(len0 + len1 + 1);
    res[0] = '\0';
    
    if (state->regs[i0]) strcat(res, state->regs[i0]);
    if (state->regs[i1]) strcat(res, state->regs[i1]);

    // Теперь безопасно удаляем старое значение
    if (state->regs[i0]) {
        free(state->regs[i0]);
    }
    
    state->regs[i0] = res;
}

void tokenize_1(State *state, char *arg) {
    if (!arg) arg = "";
    
    char *str_copy = my_strdup(arg);
    int count = 0;
    
    // Разбиваем строку по подчеркиванию
    char *token = strtok(str_copy, "_");
    while (token != NULL && count < 255) {
        count++;
        // Очищаем старое значение в регистре, если оно было
        if (state->regs[count]) free(state->regs[count]);
        state->regs[count] = my_strdup(token);
        
        token = strtok(NULL, "_");
    }
    free(str_copy);

    // Записываем количество токенов в нулевой регистр
    char buf[32];
    sprintf(buf, "%d", count);
    if (state->regs[0]) free(state->regs[0]);
    state->regs[0] = my_strdup(buf);
}