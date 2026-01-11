#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <gtk/gtk.h>

#include "filesave.h"

#define FILENAME "./save.txt"
#define FORMAT "2026-01-11 1440"

int read_today(void)
{
  FILE *fp;
  
  if ((fp = fopen(FILENAME, "r")) == NULL) {
    fprintf(stderr, "cannot open %s\n", FILENAME);
    exit(EXIT_FAILURE);
  }
  
  int len = strlen(FORMAT);
  int total_today = 0;
  time_t cur_date = time(NULL);
  char cur_date_s[12], s[len + 1];

  assert(len == 16);

  strftime(cur_date_s, sizeof(cur_date_s), "%Y-%m-%d", localtime(&cur_date));

  while(fgets(s, len + 1, fp) != NULL) {
    if (strstr(s, cur_date_s) != NULL) {
      sscanf(s, "%*d-%*d-%*d %d", &total_today);
      break;
    }
  }
  
  fclose(fp);
  return total_today;
}

void write_today(FILE *fp)
{

}