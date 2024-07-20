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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <math.h>
#include <cairo-svg.h>
#include <pango/pangocairo.h>
#include "moonwall.h"

void usage() {
	printf("%s-%s\n\n", PROG , THIS_VERSION);
	printf("\tGenerate SVG or PNG Linux wallpapers.\n");
	printf("\tThe name is as it implies: hard coded!\n\n");
	printf("Usage :\n");
	printf("%s [-n,-f,-s,-i,-c,-x,-y,-d,-b,-h,-v,-a,-p]\n", PROG);
	printf("\t-n [string] image file name\n");
	printf("\t-f [string] ttf font family - quoted if there are spaces\n");
	printf("\t-s [int] font size\n");
	printf("\t-i [string int int] \"/path/to/icon.png x y\"\n");
	printf("\t-e [int] factor for moonrise 0-100\n");
	printf("\t-c centres icon and/or text\n");
	printf("\t-x [int] width of image in pixels\n");
	printf("\t-y [int] height of image in pixels\n");
	printf("\t-d [/path/to/directory] destination directory: (default: $HOME)\n");
	printf("\t-r [int 0 - 2] 0 is default; prints the stars in a random pattern.\n"
		"\t\t\"-r1\" uses the default config to place the stars.\n"
		"\t\t\"-r2\" uses a random pattern but prints that to stdout\n"	
		"\t\tNOTE: only works with \"-b1\" option as all stars are produced.\n");	
	printf("\t-b [int 0 - 2] 0 is the default if \"-b\" is not specified\n");
	printf("\t\t0 is a moon, 1 is a sun and 2 is an eclipse\n");
	printf("\t-p : save to PNG, SVG is the default.\n");
	printf("\t-a : use a specified colour from the config for the foreground.\n");
	printf("\t-h : show this help and exit.\n");
	printf("\t-v : show version and exit.\n");
	exit (EXIT_SUCCESS);
}

struct { 
	/* allows an icon */
	cairo_surface_t *image;
}glob;

static const char *get_user_out_file(char *destination) {
	static char out_file[PATH_MAX];
	if (destination != NULL) {
		snprintf(out_file, sizeof(out_file), "%s", destination);
	} else {
		fprintf(stderr, "Failed to recognise directory\n");
		exit (EXIT_FAILURE);
	}
	mkdir(out_file, 0755);
	if (access(out_file, W_OK) == -1) {
		fprintf(stderr, "Failed to access directory %s\n", out_file);
		exit (EXIT_FAILURE);
	}
	return out_file;
	
}

static cairo_pattern_t *lpattern(int angle, int wdth, int hght, double offset,
				double r1, double g1, double b1, double r2, 
				double g2, double b2, double a, int effect) {
	cairo_pattern_t *linear;
	linear = cairo_pattern_create_linear(wdth / 2, hght / 2, wdth / 2, hght);
	cairo_pattern_add_color_stop_rgba(linear,  0.1, r1, g1, b1, a);
	cairo_pattern_add_color_stop_rgba(linear, offset, r2, g2, b2, a);
	if (effect == 1) {
		cairo_pattern_add_color_stop_rgba(linear, 0.9, r1, g1, b1, a);
	}
	return linear;
}

static PangoLayout *hlayout(const char *font, double f_size, cairo_t *c, int wdth, char *label) {
	PangoLayout *layout;
	PangoFontDescription *font_description;
		
	font_description = pango_font_description_new ();
	pango_font_description_set_family (font_description, font);
	pango_font_description_set_style (font_description, PANGO_STYLE_NORMAL ); /*PANGO_STYLE_NORMAL = 0, PANGO_STYLE_OBLIQUE = 1*/
	pango_font_description_set_weight (font_description, PANGO_WEIGHT_NORMAL); /*PANGO_WEIGHT_NORMAL = 400, PANGO_WEIGHT_BOLD = 700*/
	pango_font_description_set_absolute_size (font_description, f_size * PANGO_SCALE);
	layout = pango_cairo_create_layout (c);
	pango_layout_set_font_description (layout, font_description);
	pango_layout_set_width (layout, 9 * wdth / 20 * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD);
	pango_layout_set_text (layout, label, -1);
	pango_font_description_free (font_description);
	return layout;
}

void paint_img(char *label, const char *font, char *slabel, const char *sfont, char *tlabel, const char *tfont, char *jpos, char *kpos, char *mpos, const char *name, int wdth, int hght,
		char *dest, int rnd, int flag, int aflag, int pflag, char *iconin, int centred, int efactor, char *xcol, char *ycol, char *zcol) {
	char destimg[PATH_MAX];
	double radii = 0.025 * wdth;
	const char *fp_color = "0.505 0.317 0.215";
	double r, g , b, a;
	char red[8];
	char green[8];
	char blue[8];
	char alpha[8] = "1.0"; /* default opaque */
	char *ent = "loc"; /* config */
	double aspect;
	
	/* icon */
	char icon[PATH_MAX];
	char icon_pre[PATH_MAX];
	char posx[8];
	char posy[8];
	int icon_x, icon_y;
	if (iconin != NULL) {
		int icon_res;
		if (centred  == 0) {	
			icon_res = sscanf(iconin, "%s %s %s", icon_pre, posx, posy);
			if (icon_res < 3) {
				fprintf(stderr,"ERROR: path, x and y positions are required or use \"-c\" option\n");
				exit (EXIT_FAILURE);
			}
		} else {
			icon_res = sscanf(iconin, "%s 0 0", icon_pre);
		}
		if (icon_res > 3) {
			fprintf(stderr,"ERROR: too many arguments\n");
			exit (EXIT_FAILURE);
		}
		snprintf(icon, sizeof(icon), "%s", icon_pre);
		if (access(icon, R_OK) == -1) {
			fprintf(stderr, "Failed to access icon %s\n", icon);
			exit (EXIT_FAILURE);
		}
			icon_x = atoi(posx);
			icon_y = atoi(posy);
	}
	
	int len = strlen(fp_color);
	if (len > 32 ) {
		fprintf(stderr,"ERROR: colour argument too long\n");
		exit (EXIT_FAILURE);
	}
	int result = sscanf(fp_color, "%s %s %s %s", red, green, blue, alpha);
	if (result < 3) {
		fprintf(stderr,"ERROR: less than 3 colour aguments!\n");
		exit (EXIT_FAILURE);
	}

	/*
	 * this allows an alternate colour
	 * it can be sourced from the config file
	 */
	if (aflag == 1) {
		fp_color = parse_conf_colour("rgb");
		int len = strlen(fp_color);
		if (len > 32 ) {
			fprintf(stderr,"ERROR: colour argument too long\n");
			exit (EXIT_FAILURE);
		}
		result = sscanf(fp_color, "%s %s %s", red, green, blue);
		if (result < 3) {
			fprintf(stderr,"ERROR: less than 3 colour aguments!\n");
			exit (EXIT_FAILURE);
		}
	}

	r = atof(red);
	g = atof(green);
	b = atof(blue);
	a = atof(alpha);

	/* the flag chooses what condition we want
	 * moon = 0, for the moon, (default)
	 * sunset = 1, for the moon sunset
	 * eclipse = 2, for the eclipse
	 */
	if ((flag < 0) || flag > 2) {
		fprintf(stderr, "Error: the \"-b\" arg must be between 0 and 2 inclusive\n");
		exit (EXIT_FAILURE);
	}
	
	if ((flag == 0) || (flag == 2)) {
		r = r - 0.2;
		g = g - 0.2;
		b = b - 0.2;
	}

	cairo_surface_t *cs;
	if (pflag == 0) {
		snprintf(destimg, sizeof(destimg), "%s/%s.svg", get_user_out_file(dest), name);
		cs = cairo_svg_surface_create(destimg, wdth, hght);
	} else {
		cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, wdth, hght); /* CAIRO_FORMAT_RGB16_565  16 bit */
		snprintf(destimg, sizeof(destimg), "%s/%s.png", get_user_out_file(dest), name);
	}

	cairo_t *c;
	c = cairo_create(cs);
	cairo_pattern_t *pat;
	/* sky */
	if (strcmp(name, "desert-moonrise-wikibanner") == 0) {
		double r = (double)wdth / 25;
		cairo_move_to(c, r, 0);
		cairo_line_to(c, wdth - r, 0.0);
		cairo_curve_to (c, wdth, 0.0, wdth, 0.0, wdth, r);
		cairo_line_to(c, wdth, hght -r);
		cairo_curve_to (c, wdth, hght, wdth, hght, wdth - r, hght);
		cairo_line_to(c, r, hght);
		cairo_curve_to (c, 0.0, hght, 0.0, hght, 0.0, hght - r);
		cairo_line_to(c, 0.0, r);
		cairo_curve_to (c, 0.0, 0.0, 0.0, 0.0, r, 0.0);
		cairo_close_path(c);	
	} else if (strcmp(name, "desert-moonrise-dvdlabel") == 0) {
		cairo_arc(c, 0.5 * wdth, 0.5 * hght, 0.5 * wdth, 0, M_PI * 2);
	} else {
		cairo_rectangle(c, 0, 0, wdth, hght);		
	}
	if  (flag == 0) {
		cairo_set_source_rgb(c, 0.0, 0.0, 0.121);
		cairo_fill(c);
	}
	if  (flag == 1) {
		cairo_pattern_t *pat;
		pat = lpattern(1.0, wdth / 2, hght, 0.45, 0.305, 0.327, 0.8572, 0.6, 0.407, 0.239, a, 0);
		cairo_set_source(c, pat);
		cairo_fill(c);
		cairo_pattern_destroy(pat);
	}
	if (flag == 2) {
		cairo_set_source_rgb(c, 0.0, 0.0, 0.194);
		cairo_fill(c);
	}

	/* reference width and height */
	int rw = 480;
	int rh = 270;
	int fw = wdth / rw;
	int fh = hght / rh;
	/* this configures the moonrise */
	double wz = (wdth / 8) - (efactor * 0.01) * (wdth / 8);
	double hz = -(hght / 2.7) + (efactor * 0.01) * (hght / 2.7);

	/*
	 * rnd = 0, "-r0" (or left out) is the default, and puts the stars in a random pattern
	 * "-r1" option flips this switch so that the stars follow the pattern in "loc.conf".
	 * "loc.conf" can be in cwd, HOME or /etc.
	 * HOME is searched first, /etc next then cwd as a fallback
	 * conf = 0 is default and conflicts with rnd = 1, so "-r2" if set this will disable
	 * sourcing "loc.con" so that the random pattern is printed to stdout which you can
	 * use to create a new conf.
	 */
	if ((rnd < 0) || (rnd > 2)) {
		fprintf(stdout, "Warning: %d is an inalid value for \"-r\",\n"
			"setting to default", rnd);
		rnd = 0;
	}
	
	/* moon and stars */
	if (flag == 0) {
		int x;
		/* stars */
		for(x = 1; x < 100; x++) {
			int sx;
			int sy;
			double sz = (rand() % 100);
			sz =  0.000012 * wdth * sz;
			if ((rnd == 0) || (rnd == 2)) {
				sx = rand() % wdth;
				sy = rand() % hght;
			} else {
				sx = parse_conf(ent, x, 0) * fw;
				sy = parse_conf(ent, x, 1) * fh;
			}
			if (rnd == 2) {
				printf("loc%d = %d %d\n", x, sx, sy);
			}
			cairo_move_to(c, sx - wz, sy - hz);
			cairo_rel_curve_to(c, 5.0 * sz,      0.0, 5.0 * sz,      0.0, 5.0 * sz, -5.0 * sz);
			cairo_rel_curve_to(c,      0.0, 5.0 * sz,      0.0, 5.0 * sz, 5.0 * sz,  5.0 * sz);
			cairo_rel_curve_to(c,-5.0 * sz,      0.0,-5.0 * sz,      0.0,-5.0 * sz,  5.0 * sz);
			cairo_rel_curve_to(c,      0.0,-5.0 * sz,      0.0,-5.0 * sz,-5.0 * sz, -5.0 * sz);
			cairo_close_path(c);
			cairo_set_source_rgb(c, 1.0, 1.0, 0.9);
			cairo_fill(c);
		}

		/* pseudo gradient for moon haze */
		int i;
		double rr = 0.219;
		double gg = 0.278;
		double bb = 0.360;
		double aa = 0.05;
		int rad = radii + (0.2 * radii);
		for (i = 0; i < 100; i++) {
			cairo_arc (c, (0.5 * wdth) - wz, (0.2 * hght) - hz, rad + ((0.1 * rad) * i) , 0, M_PI * 2);
			cairo_set_source_rgba(c, rr, gg, bb, aa);
			cairo_fill(c);
			aa = aa - 0.002;
			if (aa == 0.0) {
				break;
			}
		}

		/* moon */
		pat = cairo_pattern_create_radial ((0.5 * wdth) - wz, (0.2 * hght) - hz, 110,  (0.3 * wdth) - wz, (0.3 * hght) - hz, radii);
		cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.882, 0.749, 0.498, 0.8);
		cairo_pattern_add_color_stop_rgba(pat, 0.8, 0.219, 0.278, 0.360, 0.8);
		cairo_set_source(c, pat);
		cairo_fill(c);
		cairo_arc (c, (0.5 * wdth) - wz, (0.2 * hght) - hz, radii, 0, M_PI * 2);
		cairo_fill(c);
		cairo_pattern_destroy(pat);
	} /* moon and stars */

	/* eclipe and stars */
	if (flag == 2) {
		int x;
		/* stars */
		for(x = 1; x < 50; x++) {
			int sx;
			int sy;
			double sz = (rand() % 100);
			sz =  0.00001 * wdth * sz;
			if ((rnd == 0) || (rnd == 2)) {
				sx = rand() % wdth;
				sy = rand() % hght;
			} else {
				sx = parse_conf(ent, x, 0) * fw;
				sy = parse_conf(ent, x, 1) * fh;
			}
			cairo_move_to(c, sx, sy);
			cairo_rel_curve_to(c, 4.0 * sz,      0.0, 4.0 * sz,      0.0, 4.0 * sz, -4.0 * sz);
			cairo_rel_curve_to(c,      0.0, 4.0 * sz,      0.0, 4.0 * sz, 4.0 * sz,  4.0 * sz);
			cairo_rel_curve_to(c,-5.0 * sz,      0.0,-4.0 * sz,      0.0,-4.0 * sz,  4.0 * sz);
			cairo_rel_curve_to(c,      0.0,-4.0 * sz,      0.0,-4.0 * sz,-4.0 * sz, -4.0 * sz);
			cairo_close_path(c);
			cairo_set_source_rgb(c, 1.0, 1.0, 0.9);
			cairo_fill(c);
		}
		/*sun*/
		cairo_set_line_width(c, hght / 300);
		cairo_set_source_rgba(c, 0.882, 0.549, 0.098, 0.9);
		cairo_arc (c, 0.439 * wdth, 0.369 * hght, (0.02 * radii) + radii, 0, M_PI * 2);
		cairo_fill(c);
		/* pseudo gradient */
		int i = 0;
		double rr = 0.882;
		double gg = 0.749;
		double bb = 0.098;
		double aa = 0.01;
		int rad = (0.04 * radii) + radii;
		for (i = 0; i < 20; i++) {
			cairo_arc (c, 0.439 * wdth, 0.369 * hght, rad + ((0.05 * rad) * i) , 0, M_PI * 2);
			cairo_set_source_rgba(c, rr, gg, bb, aa);
			cairo_fill(c);
			aa = aa - (0.0002 * (rad / 100));
			if (aa == 0.0) {
				break;
			}
		}
		/*moon*/
		pat = cairo_pattern_create_radial (0.44 * wdth, 0.37 * hght, 1.1 * radii,  0.47 * wdth, 0.39 * hght, radii);
		cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.062, 0.0749, 0.0498, 1);
		cairo_pattern_add_color_stop_rgba(pat, 0.8, 0.0219, 0.0278, 0.0360, 1);
		cairo_set_source(c, pat);
		cairo_fill(c);
		cairo_arc (c, 0.44 * wdth, 0.37 * hght, radii, 0, M_PI * 2);
		cairo_fill(c);
		cairo_pattern_destroy(pat);
	} /* eclipse and stars */

	/* sunset */
	if (flag == 1) {
		/* sun */
		double offset = 0.65;
		int e = 0;
		double rr = 0.882;
		double gg = 0.749;
		double bb = 0.098;
		double aa = 0.01;
		int rad = radii + (0.04 * radii);
		for (e = 0; e < 60; e++) {
			cairo_arc (c,0.384 * wdth, 0.530 * hght, rad + ((0.05 * rad) * e) , 0, M_PI * 2);
			cairo_set_source_rgba(c, rr, gg, bb, aa);
		cairo_fill(c);
			aa = aa - (0.0002 * (rad / 100));
			if (aa == 0.0) {
			break;
			}
		}
		pat = cairo_pattern_create_radial (0.384 * wdth, 0.530 * hght, 1.10 * radii,  0.384 * wdth, 0.530 * hght, radii * 3);
		cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.882, 0.749, 0.098, .1);
		cairo_pattern_add_color_stop_rgba(pat, offset * 1, 0.019, 0.278, 0.360, 0.1);
		cairo_set_source(c, pat);
		cairo_fill(c);
		cairo_pattern_destroy(pat);
		pat = cairo_pattern_create_radial (0.384 * wdth, 0.530 * hght, 0.75 * radii,  0.384 * wdth, 0.530 * hght, radii + (0.1 * radii));
		cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.784, 0.541, 0.168, 0.1);
		cairo_pattern_add_color_stop_rgba(pat, offset, 0.882, 0.749, 0.098, 0.1);
		cairo_set_source(c, pat);
		cairo_arc (c, 0.384 * wdth, 0.530 * hght, radii + (0.1 * radii), 0, M_PI * 2);
		cairo_fill(c);
		cairo_pattern_destroy(pat);
		pat = cairo_pattern_create_radial (0.384 * wdth, 0.530 * hght, 0.75 * radii,  0.384 * wdth, 0.530 * hght, radii);
		cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.784, 0.541, 0.168, 1.0);
		cairo_pattern_add_color_stop_rgba(pat, offset, 0.882, 0.749, 0.098, 0.6);
		cairo_set_source(c, pat);
		cairo_arc (c, 0.384 * wdth, 0.530 * hght, radii, 0, M_PI * 2);
		cairo_fill(c);
		cairo_pattern_destroy(pat);

	} /* sunset */
	
	/* text for main label */
	if (label) {
		int msg_len = strlen(label);
		if (msg_len > 76) {
			fprintf(stderr,"\"%s\" is too long!\n", label);
			exit (EXIT_FAILURE);
		}
		
		char fontfam[32];
		char fontsz[4];
		int fresult = sscanf(font, "%s %s", fontfam, fontsz);
		if (fresult < 2) {
			fprintf(stderr,"ERROR: less than 2 font arguments!\n");
			exit (EXIT_FAILURE);
		}
		double font_sz = atof(fontsz);
		
		/* font color */
		char xred[8];
		char xgreen[8];
		char xblue[8];
		char xalpha[8];
		double xr;
		double xb;
		double xg;
		double xa;
		if (xcol) {
			int xlen = strlen(xcol);
			if (xlen > 38 ) {
				fprintf(stderr,"ERROR: colour argument too long\n");
				exit (EXIT_FAILURE);
			}
			int xresult = sscanf(xcol, "%s %s %s %s", xred, xgreen, xblue, xalpha);
			if (xresult < 4) {
				fprintf(stderr,"ERROR: less than 4 colour aguments!\n");
				exit (EXIT_FAILURE);
			}	
		}
		xr = atof(xred);
		xg = atof(xgreen);
		xb = atof(xblue);
		xa = atof(xalpha);
		
		/* font */
		PangoLayout *layout;
		layout = hlayout(fontfam, font_sz, c, wdth, label);
		
		/* position of text */
		int xposi, yposi;
		if (jpos) {	
			char prex[8];
			char prey[8];
			int font_pos = sscanf(jpos, "%s %s", prex, prey);
			if (font_pos < 2) {
				fprintf(stderr,"ERROR: x and y positions are required\n");
				exit (EXIT_FAILURE);
			}
			if (font_pos > 2) {
				fprintf(stderr,"ERROR: too many args\n");
				exit (EXIT_FAILURE);
			}
			xposi = atoi(prex);
			yposi = atoi(prey);
		} else if (centred == 1) {
			int wrect, hrect;
			PangoRectangle rect = { 0 };
			pango_layout_get_extents(layout, NULL, &rect);
			pango_extents_to_pixels(&rect, NULL);
			wrect = rect.width;
			wrect = ((wrect) < (wdth)) ? (wrect) : (wdth);
			hrect = rect.height;
			pango_layout_set_alignment(layout, wrect * PANGO_ALIGN_CENTER);
			pango_layout_set_width(layout, wrect * PANGO_SCALE);
			if ((!iconin) || (!slabel)) {
				xposi = (wdth / 2) - (wrect / 2);
				yposi = (hght / 2) - (hrect / 2);
			} else {
				xposi = (wdth / 2) - (wrect / 2);
				yposi = (hght / 2) + 10;
			}
		
		} else { /* fallback */
			xposi = wdth / 2;
			yposi = 3 * hght / 7;
		}

		cairo_move_to(c, xposi - wz , 1 * yposi -hz);
		cairo_set_source_rgba(c, xr, xg, xb, xa);
		pango_cairo_show_layout (c, layout);
		g_object_unref (layout);
		
		if (slabel) { /* text for 2nd label */
			char yred[8];
			char ygreen[8];
			char yblue[8];
			char yalpha[8];
			double yr;
			double yb;
			double yg;
			double ya;
			if (ycol) {
				int ylen = strlen(ycol);
				if (ylen > 38 ) {
					fprintf(stderr,"ERROR: colour argument too long\n");
					exit (EXIT_FAILURE);
				}
				int yresult = sscanf(ycol, "%s %s %s %s", yred, ygreen, yblue, yalpha);
				if (yresult < 4) {
					fprintf(stderr,"ERROR: less than 4 colour aguments!\n");
					exit (EXIT_FAILURE);
				}	
			}
			yr = atof(yred);
			yg = atof(ygreen);
			yb = atof(yblue);
			ya = atof(yalpha);
			
			char sfontfam[32];
			char sfontsz[4];
			int sfresult = sscanf(sfont, "%s %s", sfontfam, sfontsz);
			if (sfresult < 2) {
				fprintf(stderr,"ERROR: less than 2 font arguments!\n");
				exit (EXIT_FAILURE);
			}
			double sfont_sz = atof(sfontsz);
			PangoLayout *slayout;
			slayout = hlayout(sfontfam, sfont_sz, c, wdth, slabel);
			int xsposi;
			int ysposi;
			if (kpos) {
				char sprex[8];
				char sprey[8];
				int font_spos = sscanf(kpos, "%s %s", sprex, sprey);
				if (font_spos < 2) {
					fprintf(stderr,"ERROR: x and y positions are required\n");
					exit (EXIT_FAILURE);
				}
				if (font_spos > 2) {
					fprintf(stderr,"ERROR: %d too many args\n", font_spos);
					exit (EXIT_FAILURE);
				}
				xsposi = atoi(sprex);
				ysposi = atoi(sprey);
			} else if (centred == 1) {
				int wrect, hrect;
				PangoRectangle rect = { 0 };
				pango_layout_get_extents(slayout, NULL, &rect);
				pango_extents_to_pixels(&rect, NULL);
				wrect = rect.width;
				wrect = ((wrect) < (wdth)) ? (wrect) : (wdth);
				hrect = rect.height;
				pango_layout_set_width(slayout, wrect * PANGO_SCALE);
				xsposi = (wdth / 2) - (wrect / 2);
				ysposi = (hght / 2) + (hrect / 2);
		
			} else { /* fallback */
				xsposi = wdth / 2;
				ysposi = (3 * hght / 7) + sfont_sz + 10;
			}

			cairo_move_to(c, xsposi - wz , 1 * ysposi -hz);
			cairo_set_source_rgba(c, yr, yg, yb, ya);
			pango_cairo_show_layout (c, slayout);
		}
	}
	
	/* foreground */
	/*
	 * Aspect ratios
	 * 1280x1024 or 2560x2048 5:4 SXGA
	 * 1600x1200 or 2048x1536 4:3 standard
	 * 1920x1200 or 2560x1600 16:10 widescreen
	 * 1920x1080 16:9 FullHD
	 *
	 */
	if (strcmp(name, "desert-moonrise-dvdlabel") != 0) {
		int ahght;
		int awdth;
		double wf;
		double sp;
		double fp;
		aspect = (double)wdth / (double)hght;
		if (aspect == 1.25) { /* SXGA */
			wf =  wdth * 0.04;
			sp = 0.0 - (7.0 * wf);  //-250.0;
			fp = wdth + (2 * wf);
			awdth = fp;
			ahght = hght;
			cairo_scale(c, 1.32, 1.0);
			cairo_translate(c, (sp / 1.7), 0.0);
			ahght = hght;
		} else if ((aspect >= 1.3) && (aspect <= 1.34)) { /* VGA */
			wf =  wdth * 0.02;
			sp = 0.0 - (7.6 * wf);        //-300; //0.0 - (2.0 * wf);  //-250.0;
			fp = wdth + (2 * wf);
			awdth = fp;
			ahght = hght;
			cairo_scale(c, 1.25, 1.0);
			cairo_translate(c, (sp / 1.3), 0.0);
		} else if (aspect == 1.6) { /* widescreen */
			wf =  wdth * 0.038;
			sp = 0.0 - (2.0 * wf);  //-250.0;
			fp = wdth + (2 * wf);
			awdth = fp;
			ahght = hght;
			cairo_translate(c, (sp / 2), 0.0);
		} else if ((aspect >= 1.7) || (aspect <= 1.78)) { /* FHD */
			wf =  wdth;
			sp = 0.0;
			fp = wdth;
			awdth = wdth;
			ahght = hght;
		}

		double i = 0;
		for (i = 0; i < 80; i++) {
			double l = 0.00333 * i;
			double p = 0.0027 * i;
			cairo_move_to(c, sp, (0.4667 + l) * ahght);
			cairo_curve_to(c, 0.1 * awdth, (0.4667 + l) * ahght, 0.2 * awdth, (0.4967 + l) * ahght, 0.3 * awdth, (0.5167 + l) * ahght);
			cairo_curve_to(c, 0.4 * awdth, (0.5367 + l) * ahght, 0.5 * awdth, (0.5967 + l) * ahght, 0.6 * awdth, (0.4867 + l) * ahght);
			cairo_curve_to(c, 0.75 * awdth, (0.3267 + l) * ahght, 0.85 * awdth, (0.3867 + l) * ahght, 1.0 * awdth, (0.3867 + l) * ahght);
			cairo_line_to(c, fp, 1.0 * ahght);		
			cairo_line_to(c, sp, 1.0 * ahght);
			cairo_close_path(c);
			cairo_set_source_rgba(c, r - p, g - p, b - p, a);
			cairo_fill(c);
		}
		i = 0;
		for (i = 0; i < 130; i++) {
			double l = 0.00333 * i;
			double q = 0.0037 * i;
			cairo_move_to(c, sp, (0.6667 + l) * ahght);
			cairo_curve_to(c, 0.22 * awdth, (0.7667 + l) * ahght, 0.33 * awdth, (0.8067 + l) * ahght, 0.55 * awdth, (0.7167 + l) * ahght);
			cairo_curve_to(c, 0.7 * awdth, (0.6367 + l) * ahght, 0.8 * awdth, (0.6167 + l) * ahght, 1.0 * awdth, (0.5667 + l) * ahght);
			cairo_line_to(c, fp, 1.0 * ahght);
			cairo_line_to(c, sp, 1.0 * ahght);
			cairo_close_path(c);
			cairo_set_source_rgba(c, r - q, g - q, b - q, a);
			cairo_fill(c);
		} 
	} else if (strcmp(name, "desert-moonrise-dvdlabel") == 0) {
		pat = lpattern(20.0, wdth / 2, hght, 0.45, 0.405, 0.217, 0.115, 0.315, 0.127, 0.0555, 1.0, 0);
		cairo_move_to(c, 0.0, 0.5 * hght);
		cairo_curve_to(c, 0.25 * wdth, 0.5067 * hght, 0.36 * wdth, 0.667 * hght, 0.6 * wdth, 0.5 * hght);
		cairo_curve_to(c, 0.69 * wdth, 0.4367 * hght, 0.79 * wdth, 0.4467 * hght, wdth, 0.5 * hght);
		cairo_arc (c, 0.5 * wdth, 0.5 * hght, 0.5 * wdth, M_PI * 2, M_PI);
		cairo_close_path(c);
		cairo_set_source(c, pat);
		cairo_fill(c);
		cairo_pattern_destroy(pat);
	} /* foreground */
	
	if (tlabel) {
		char zred[8];
		char zgreen[8];
		char zblue[8];
		char zalpha[8];
		double zr;
		double zb;
		double zg;
		double za;
		if (zcol) {
			int zlen = strlen(zcol);
			if (zlen > 38 ) {
				fprintf(stderr,"ERROR: colour argument too long\n");
				exit (EXIT_FAILURE);
			}
			int zresult = sscanf(zcol, "%s %s %s %s", zred, zgreen, zblue, zalpha);
			if (zresult < 4) {
				fprintf(stderr,"ERROR: less than 4 colour aguments!\n");
				exit (EXIT_FAILURE);
			}	
		}
		zr = atof(zred);
		zg = atof(zgreen);
		zb = atof(zblue);
		za = atof(zalpha);char tfontfam[32];
		char tfontsz[4];
		int tfresult = sscanf(tfont, "%s %s", tfontfam, tfontsz);
		if (tfresult < 2) {
			fprintf(stderr,"ERROR: less than 2 font arguments!\n");
			exit (EXIT_FAILURE);
		}
		double tfont_sz = atof(tfontsz);
		PangoLayout *tlayout;
		tlayout = hlayout(tfontfam, tfont_sz, c, wdth, tlabel);
		int xtposi;
		int ytposi;
		if (mpos) {
			char tprex[8];
			char tprey[8];
			int font_tpos = sscanf(mpos, "%s %s", tprex, tprey);
			if (font_tpos < 2) {
				fprintf(stderr,"ERROR: x and y positions are required\n");
				exit (EXIT_FAILURE);
			}
			if (font_tpos > 2) {
				fprintf(stderr,"ERROR: %d too many args\n", font_tpos);
				exit (EXIT_FAILURE);
			}
			xtposi = atoi(tprex);
			ytposi = atoi(tprey);
		} else if (centred == 1) {
			int wrect, hrect;
			PangoRectangle rect = { 0 };
			pango_layout_get_extents(tlayout, NULL, &rect);
			pango_extents_to_pixels(&rect, NULL);
			wrect = rect.width;
			wrect = ((wrect) < (wdth)) ? (wrect) : (wdth);
			hrect = rect.height;
			pango_layout_set_width(tlayout, wrect * PANGO_SCALE);
			xtposi = (wdth / 2) - (wrect / 2);
			ytposi = (hght / 2) + (hrect / 2);
	
		} else { /* fallback */
			xtposi = wdth / 2;
			ytposi = (3 * hght / 5) + tfont_sz + 10;
		}
		cairo_move_to(c, xtposi - wz , 1 * ytposi -hz);
		cairo_set_source_rgba(c, zr, zg, zb, za);
		pango_cairo_show_layout (c, tlayout);
	}
	
	/* icon and position */
	if (iconin != NULL) {
		glob.image = cairo_image_surface_create_from_png(icon);
		int wx = cairo_image_surface_get_width(glob.image);
		int hy = cairo_image_surface_get_height(glob.image);
		if ((centred == 1) && (!label)) {
			icon_x = (wdth / 2) - (wx / 2);
			icon_y = (hght / 2) - (hy / 2);
		} else if ((centred == 1) && (label)){
			icon_x = (wdth / 2) - (wx / 2);
			icon_y = (hght / 2) - hy;			
		}
		cairo_set_source_surface(c, glob.image, icon_x, icon_y);
		cairo_paint(c);
	}
	/* cleanup */
	cairo_status_t res = cairo_surface_status(cs);
	const char *ret;
	ret = cairo_status_to_string (res);
	if (res != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(cs);
		cairo_destroy(c);
		fprintf(stderr, "Cairo : %s\n", ret);
		exit (EXIT_FAILURE);
	}
	if (pflag == 1) {
		cairo_surface_write_to_png (cs, destimg);
	}
	cairo_surface_destroy(cs);
	cairo_destroy(c);
	if (rnd != 2) {
		fprintf(stdout, "image saved to %s\n", destimg);
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage();
		return 0;
	}
	char *Lvalue = "debian"; /* main label for image */
	char *Fvalue = "Poppl-Laudatio 40"; /* 1st font and size */
	//int F_size = 40; /* font size */
	char *jvalue = NULL; /* 1st font x y */
	char *lvalue = NULL; /* secondary label for image */
	char *fvalue = "font-logos 60"; /* 2nd font and size */
	char *kvalue = NULL; /* 2nd font x y */
	//int f_size = 60; /* font size, usually a logo font */
	char *tvalue = NULL; /* tertiary label for image */
	char *Tvalue = NULL; /* 3rd font and size */
	char *mvalue = NULL; /* 3rd font x y */
	char *nvalue = "foo"; /* image name */
	char *dvalue = getenv("HOME");
	char *Xval = "1.0 1.0 1.0 0.8"; /* 1st font color */
	char *Yval = "1.0 1.0 1.0 0.5"; /* 2nd font color */
	char *Zval = "1.0 1.0 1.0 0.8"; /* 3rd font color */
	char *ivalue = NULL; /* embedded icon */
	int width = 480; int height = 270;
	int efact = 100; /* 0 - 200 for moonrise - plymouth animation */
	int rflag = 0; /* config options */
	int bflag = 0; /* sets the mode, moon, sun, eclipse */
	int pflag = 0; /* save as png */
	int aflag = 0; /* change foreground color from config */
	int cflag = 0; /* centred text/icon */
	int vflag = 0; 
	int hflag = 0;
	int c;
	while ((c = getopt (argc, argv, "L:F:l:f:T:t:j:k:m:n:x:y:d:r:b:i:e:X:Y:Z:hvapc")) != -1) {
		switch (c)
		{
			case 'L':
				Lvalue = optarg;
				break;
			case 'F':
				Fvalue = optarg;
				break;
			case 'T':
				Tvalue = optarg;
				break;
			case 'l':
				lvalue = optarg;
				break;
			case 'f':
				fvalue = optarg;
				break;
			case 't':
				tvalue = optarg;
				break;
			case 'j':
				jvalue = optarg;
				break;
			case 'k':
				kvalue = optarg;
				break;
			case 'm':
				mvalue = optarg;
				break;
			case 'n':
				nvalue = optarg;
				break;
			case 'x':
				width = atoi(optarg);
				break;
			case 'y':
				height = atoi(optarg);
				break;
			case 'd':
				dvalue = optarg;
				break;
			case 'r':
				rflag = atoi(optarg);
				break;
			case 'b':
				bflag = atoi(optarg);
				break;
			case 'a':
				aflag = 1;
				break;
			case 'i':
				ivalue = optarg;
				break;
			case 'e':
				efact = atoi(optarg);
				break;
			case 'p':
				pflag = 1;
				break;
			case 'c':
				cflag = 1;
				break;
			case 'X':
				Xval = optarg;
				break;
			case 'Y':
				Yval = optarg;
				break;
			case 'Z':
				Zval = optarg;
				break;
			case 'h':
				hflag = 1;
				if (hflag == 1) usage();
				break;
			case 'v':
				vflag = 1;
				if (vflag == 1) fprintf(stdout, "%s\n", THIS_VERSION);
				return 0;
			default:
				usage();
		}
	}
	paint_img(Lvalue, Fvalue, lvalue, fvalue, tvalue, Tvalue, jvalue, kvalue, mvalue, nvalue, width, height, dvalue, rflag, bflag, aflag, pflag, ivalue, cflag, efact, Xval, Yval, Zval);
	return 0;
}
