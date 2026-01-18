#define FILENAME "./save.txt"
#define FORMAT "yyyy-mm-dd xxxx\n"
#define UPD_FREQ 10

/* Retrieves today's work time in minutes from save file */
short read_today(void);

/* Updates today's work time in save file */
void write_today(short new_time);