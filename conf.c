/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * (c)2015 - 2024 Michael Amadio. Gold Coast QLD, Australia 01micko@gmail.com
 */
 
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "moonwall.h"

#define CONFIG      "loc.conf"
#define USERCONFDIR ".config/moonwall"
#define SYSCONFDIR  "/etc/moonwall"
#define MYHOME      getenv("HOME")

char colorstring[34];
//const char *myhome = "HOME";
//const char *home = getenv(myhome);

char *split_string(char *var) {
	char *buf = var;
	const char s[2] = "=";
	char *token;
	token = strtok(buf, s);
	token = strtok(NULL, s);
	return token;
}

int parse_conf(char *entry, int iter, int orient) {
	/* parse the config file */
	int out;
	FILE *fp;
	char config[PATH_MAX];
	char cwd[PATH_MAX];
	int res;
	if (MYHOME) {
		res = snprintf(config, sizeof(config), "/%s/%s/%s", MYHOME, USERCONFDIR, CONFIG);
		if (res < 0) {
			fprintf(stderr, "Warning: No %s %s available\n", MYHOME, CONFIG);
		}
	} else if (!MYHOME) {
		res = snprintf(config, sizeof(config), "/%s/%s", SYSCONFDIR, CONFIG);
		if (res < 0) {
			fprintf(stderr, "Warning: No %s %s available\n", SYSCONFDIR, CONFIG);
		}
	} else if (getcwd(cwd, sizeof(cwd)) != NULL) {
		res = snprintf(config, sizeof(config), "%s/%s", cwd, CONFIG);
		if (res < 0) {
			fprintf(stderr, "Warning: No config available\n");
			return -1;
		}
	} else {
		return -1;
	}
	char line[30];
	char xxx[14];
	char yyy[14];
	int x;
	int y;
	fp = fopen(config, "r");
	if (fp) {
		int count = 0;
		while (fgets(line, sizeof(line), fp)) {
			if (strstr(line, "#") != NULL) {
				continue; /* dodge comments */
			}
			if (strstr(line, entry) != NULL) {
				char *res = split_string(line);
				if (!res) {
					fprintf(stderr, "ERROR 1: malformed config!\n");
					return -1;
				}
				int result = sscanf(res, "%s %s", xxx, yyy);
				if (result < 2 || result > 2) {
					fprintf(stderr, "ERROR 2: malformed config!\n");
					return -1;
				}
				x = atoi(xxx);
				y = atoi(yyy);
				if (iter == count) {
					if (orient == 0) {
						out = x;
					} else {
						out = y;
					}
				}
			}
			count++ ;
		}
	} else {
		fprintf(stderr,"ERROR -1: no config found\n");
		return -1;
	} 
	fclose(fp);
	return out;
}

const char *parse_conf_colour(char *entry) {
	char line[30];
	FILE *fp;
	char config[PATH_MAX];
	char *defaultstring = "0.5 0.5 0.5";
	char cwd[PATH_MAX];
	int res;
	if (MYHOME) {
		res = snprintf(config, sizeof(config), "/%s/%s/%s", MYHOME, USERCONFDIR, CONFIG);
		if (res < 0) {
			fprintf(stderr, "Warning: No %s %s available\n", MYHOME, CONFIG);
		}
	} else if (!MYHOME) {
		res = snprintf(config, sizeof(config), "/%s/%s", SYSCONFDIR, CONFIG);
		if (res < 0) {
			fprintf(stderr, "Warning: No %s %s available\n", SYSCONFDIR, CONFIG);
		}
	} else if (getcwd(cwd, sizeof(cwd)) != NULL) {
		int res = snprintf(config, sizeof(config), "%s/%s", cwd, CONFIG);
		if (res < 0) {
			printf("Warning: No config available\n");
			return defaultstring;
		}
	} else {
		return defaultstring;
	}
	fp = fopen(config, "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp)) {
			if (strstr(line, "#") != NULL) {
				continue; /* dodge comments */
			}
			if (strstr(line, "loc") != NULL) {
				continue; /* dodge locations */
			}
			if (strstr(line, entry) != NULL) {
				char *res = split_string(line);
				if (!res) {
					fprintf(stderr, "ERROR 3:  malformed config!\n");
					return defaultstring;
				}
				strcpy(colorstring, res);
			}
		}
	} else {
		fprintf(stderr,"ERROR -1: no config found\n");
		return defaultstring;
	} 
	fclose(fp);
	return colorstring;
}
