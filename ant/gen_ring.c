/* Test generator: cities on a noisy ring around the origin, uniform-ish prizes. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    double C = 0.01;
    double D = 1.0001;

    srand(time(NULL));

    printf("%d %.6f %.6f\n", N, C, D);

    double target_sum = 36e9;
    double avg_p = target_sum / N;

    for (int i = 0; i < N; i++) {
        double angle = 2.0 * M_PI * i / N;
        double radius = 5000.0 + (rand() % 5000);

        double x = radius * cos(angle);
        double y = radius * sin(angle);

        double p = avg_p * (0.8 + 0.4 * (double)rand() / RAND_MAX);

        printf("%.1f %.1f %.1f\n", x, y, p);
    }

    return 0;
}
