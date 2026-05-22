#include <stdio.h>

typedef long long Value;

static int read_value(FILE *stream, Value *number)
{
    return fscanf(stream, "%lld", number) == 1;
}

static Value select_possible_majority(FILE *stream, int amount)
{
    Value leader = 0;
    int balance = 0;

    for (int left = amount; left > 0; --left) {
        Value current;
        read_value(stream, &current);

        if (balance < 1) {
            leader = current;
            balance = 1;
            continue;
        }

        balance += (current == leader) ? 1 : -1;
    }

    return leader;
}

static int count_occurrences(FILE *stream, int amount, Value target)
{
    int matches = 0;

    for (int pos = 0; pos < amount; ++pos) {
        Value current;
        read_value(stream, &current);
        matches += (current == target);
    }

    return matches;
}

int main(void)
{
    FILE *input = fopen("input.txt", "r");
    FILE *output = fopen("output.txt", "w");

    if (input == NULL || output == NULL) {
        if (input != NULL) {
            fclose(input);
        }
        if (output != NULL) {
            fclose(output);
        }
        return 1;
    }

    Value raw_size;
    read_value(input, &raw_size);

    int size = (int)raw_size;
    Value candidate = select_possible_majority(input, size);

    rewind(input);
    read_value(input, &raw_size);

    int frequency = count_occurrences(input, size, candidate);

    if (frequency * 2 > size) {
        fprintf(output, "YES\n%lld\n", candidate);
    } else {
        fprintf(output, "NO\n");
    }

    fclose(input);
    fclose(output);
    return 0;
}
