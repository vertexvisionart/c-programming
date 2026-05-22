/* Stress generator: 4 distribution types — clustered, grid, uniform, spiral. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <N> <type>\n", argv[0]);
        fprintf(stderr, "Types: 1=clustered, 2=grid, 3=random, 4=spiral\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int type = atoi(argv[2]);
    double C = 0.01;
    double D = 1.0001;

    srand(time(NULL) + type);

    printf("%d %.6f %.6f\n", N, C, D);

    double target_sum = 36e9;
    double avg_p = target_sum / N;

    for (int i = 0; i < N; i++) {
        double x, y, p;

        switch (type) {
            case 1: {
                int cluster = i % 100;
                double cx = (cluster % 10) * 2000 - 9000;
                double cy = (cluster / 10) * 2000 - 9000;
                x = cx + (rand() % 400 - 200);
                y = cy + (rand() % 400 - 200);
                break;
            }
            case 2: {
                int side = (int)sqrt(N);
                x = (i % side) * 20 - side * 10;
                y = (i / side) * 20 - side * 10;
                break;
            }
            case 3: {
                x = rand() % 20000 - 10000;
                y = rand() % 20000 - 10000;
                break;
            }
            case 4: {
                double angle = 0.1 * i;
                double radius = 10 * i / N * 10000;
                x = radius * cos(angle);
                y = radius * sin(angle);
                break;
            }
            default:
                x = y = 0;
        }

        p = avg_p * (0.5 + 1.0 * (double)rand() / RAND_MAX);

        printf("%.1f %.1f %.1f\n", x, y, p);
    }

    return 0;
}
