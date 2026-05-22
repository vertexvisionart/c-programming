#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
//--target=x86_64-pc-linux-gnu
int main(void) {
  FILE *fin = fopen("input.txt", "r");
  FILE *fout = fopen("output.txt", "w");

  char buffer[200];
  int i = 0;

  if (fgets(buffer, sizeof(buffer), fin)) {
    while (buffer[i] != '\0') {
      while (buffer[i] == ' ' || buffer[i] == '\n') {
        i++;
      }
      if (buffer[i] == '\0') {
        break;
      }
      fputc('[', fout);

      if (buffer[i] == '"') {
        i++;
        while (buffer[i] != '\0' && buffer[i] != '"') {
          fputc(buffer[i], fout);
          i++;
        }
        if (buffer[i] == '"') {
          i++;
        }
      } else {
        while (buffer[i] != '\0' && buffer[i] != ' ' && buffer[i] != '\n') {
          fputc(buffer[i], fout);
          i++;
        }
      }
      fputc(']', fout);
      fputc('\n', fout);
    }
  }
  fclose(fin);
  fclose(fout);
  return 0;
}
