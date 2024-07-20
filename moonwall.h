#ifndef __WALL_H
#define __WALL_H

#define PROG "moonwall"
#define THIS_VERSION "0.1.0"

char *split_string(char *var);
char *find_conf(void);
extern int parse_conf(char *entry, int iter, int orient);
extern const char *parse_conf_colour(char *entry);

#endif /* __WALL_H  */
