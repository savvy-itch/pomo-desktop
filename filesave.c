#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "filesave.h"

/* Reallocates more memory for buffer array */
static char **expand_buf(char ***buf, const int new_size);
/* Frees the allocated memory for buffer array */
static void cleanup(char ***buf, const int size);

short read_today(void)
{
  FILE *fp;
  short total_today = 0;
  
  if ((fp = fopen(FILENAME, "rb")) == NULL) {
    return total_today;
  }
  
  int len = strlen(FORMAT);
  time_t cur_date = time(NULL);
  char cur_date_s[11], s[len + 1];

  assert(len == 16);

  strftime(cur_date_s, sizeof(cur_date_s), "%Y-%m-%d", localtime(&cur_date));

  while(fgets(s, len + 1, fp) != NULL) {
    // if today's string exist
    if (strstr(s, cur_date_s) != NULL) {
      sscanf(s, "%*d-%*d-%*d %hd", &total_today);
      break;
    }
  }
  
  fclose(fp);
  return total_today;
}

/*
if save file exists, rewrite is required after reading, hence rb+ mode first, then wb+ mode. If file doesn't exist, there's nothing to read, therefore wb+ is sufficient.
*/
void write_today(short new_time)
{
  FILE *fp;
  bool needs_rewrite = true;
  
  // if file doesn't exist
  if ((fp = fopen(FILENAME, "rb+")) == NULL) {
    // create one
    fp = fopen(FILENAME, "wb+");
    needs_rewrite = false;
  }

  int len = strlen(FORMAT);
  time_t cur_date = time(NULL);
  char cur_date_s[12], s[len + 1];
  
  assert(len == 16);

  strftime(cur_date_s, sizeof(cur_date_s), "%Y-%m-%d", localtime(&cur_date));

  short buf_size = 5;
  char **buf;
  
  if ((buf = g_try_malloc(sizeof (char *) * buf_size)) == NULL) {
    g_print("Failed to save progress. Insufficient memory\n");
    fclose(fp);
    return;
  }
  
  int i = 0;
  int buf_capacity = buf_size;
  bool today_str_exist = false;

  // if file is empty, doesn't contain today's string, or until the today's string is found
  while(fgets(s, len + 1, fp) != NULL) {
    if (i+1 > buf_capacity) {
      buf_capacity += buf_size;
      if (!expand_buf(&buf, buf_capacity)) {
        cleanup(&buf, i);
        fclose(fp);
        return;
      }
    }

    // if today's string exist
    if (strstr(s, cur_date_s) != NULL) {
      // retrieve the old value
      short total_today = 0;
      sscanf(s, "%*d-%*d-%*d %hd\n", &total_today);
      
      // time didn't change
      if (total_today == new_time) {
        cleanup(&buf, i);
        fclose(fp);
        return;
      }
      
      // update the string
      char *updated_s;
      if ((updated_s = g_try_malloc(len + 1)) == NULL) {
        g_print("Failed to save progress. Insufficient memory\n");
        cleanup(&buf, i);
        fclose(fp);
        return;
      }
      // sprintf(updated_s, "%s %4.4hd\n", cur_date_s, new_time);
      snprintf(updated_s, len + 1, "%.10s %4.4hd\n", cur_date_s, new_time);
      today_str_exist = true;
      // puts(updated_s);
      buf[i++] = updated_s;
    } else {
      // store string in buf
      char *f_s;
      if ((f_s = g_try_malloc(len + 1)) == NULL) {
        g_print("Failed to save progress. Insufficient memory\n");
        cleanup(&buf, i);
        fclose(fp);
        return;
      }

      strcpy(f_s, s);
      buf[i++] = f_s;
    }
  }

  // if file is empty
  if (!today_str_exist) {
    char *updated_s;
    if ((updated_s = g_try_malloc(len + 1)) == NULL) {
      g_print("Failed to save progress. Insufficient memory\n");
      cleanup(&buf, i);
      fclose(fp);
      return;
    }
    // sprintf(updated_s, "%s %4.4hd\n", cur_date_s, new_time);
    snprintf(updated_s, len + 1, "%.10s %4.4hd\n", cur_date_s, new_time);
    // puts(updated_s);
    buf[i++] = updated_s;
  }
  
  
  if (needs_rewrite) {
    fclose(fp);
    fp = fopen(FILENAME, "wb+");
  } else {
    rewind(fp);
  }

  // update save file
  for (int j = 0; j < i; j++) {
    int len = strlen(buf[j]);
    fwrite(buf[j], 1, len, fp);
  }

  // clear the buffer
  cleanup(&buf, i);
  // g_print("Updated progress\n");
  fclose(fp);
}

static char **expand_buf(char ***buf, const int new_size)
{
  char **new_buf;
  if ((new_buf = g_try_realloc(*buf, sizeof (char *) * new_size)) == NULL) {
    g_print("Failed to expand records size\n");
    return NULL;
  }

  *buf = new_buf;
  return *buf;
}

static void cleanup(char ***buf, const int size)
{
  for (int j = 0; j < size; j++) {
    g_free((*buf)[j]);
  }
  g_free(*buf);
}