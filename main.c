/*
 * Copyright 2010-2019 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pie.h"

cairo_status_t wr(void *closure, const unsigned char *data, unsigned int length)
{
	FILE *out = closure;

	fwrite(data, 1, length, out);

	return CAIRO_STATUS_SUCCESS;
}

void usage() {
	printf(
		"\n"
		"Syntax: pie -o <file> [-b <hex>] [-c <float>] [-C <hex>] [-d <float>]\n"
		"            [-e <float>] [-f <EPS|PNG|PDF|SVG>] [-h <integer>] [-i <file>]\n"
		"            [-l <hex>] [-L <float>] [-m <float>] [-r <float>] [-s <size>]\n"
		"            [-t <title>] [-T <hex>] [-w <float>] [val [val [val [...]]]]\n"
		"\n"
		" -b <hex>             : Background color (ex: #ffffff). Default is\n"
		"                        transparent\n"
		" -c <float>           : Pie line width. Default is 0\n"
		" -C <hex>             : Pie line color. Default is black\n"
		" -d <float>           : Percent explode.\n"
		"                        Values must between 0 and 1. default is 0.1\n"
		" -e <float>           : Percent extrusion (pie height).\n"
		"                        Values must between 0 and 1. Default is 0.4\n"
		" -f <EPS|PNG|PDF|SVG> : Choose output format. Default is PNG\n"
		" -h <integer>         : Height in pixel. Default is equal than -w. If\n"
		"                        -h is not defined, is 400px\n"
		" -i <file>            : Input data file\n"
		" -l <hex>             : Legend color (ex: #ffffff). Default is black\n"
		" -L <float>           : Legend size in px. Default is 10px\n"
		" -m <float>           : Margin in pixel. Deafult is 10px\n"
		" -o <file>            : Output file name. '-' is stdout\n"
		" -r <float>           : Ratio between height and width of pie.\n"
		"                        Values must between 0 and 1. default is 0.5\n"
		" -s <size>            : Title size in px. Default is 15px\n"
		" -t <title>           : Graph title. Default is empty\n"
		" -T <hex>             : Title color (ex: #ffffff). Default is black\n"
		" -w <float>           : Width in pixel. Default is equal than -h. If\n"
		"                        -h is not defined, is 400px\n"
		"\n"
		"Values format is: value<float>#color<hex>:ratio_explode<float>:name\n"
		"\n"
		"Exemple:\n"
		"  pie -f PNG -w 400 -h 400 -o toto.png -l '#000000' -L 10 -t 'the title' \\\n"
		"        -b '#ffffff' -i data -r 0.5 \"1#ff0000:0:application a\" \\\n"
		"        \"2#00ff00:0.1:application b\" \"1#0000ff:0:application c\"\n"
		"\n"

	);
	exit(0);
}

static inline void get_one(int *nb, int argc)
{
	(*nb)++;
	if ((*nb) >= argc) {
		fprintf(stderr, "argument expected\n");
		exit(1);
	}
}

static inline void add_data(char *in, struct conf *co)
{
	double value;
	double extrude;
	char color[7];
	char *name;
	char *p;
	int i;

	/* first part: part of pie */
	value = strtod(in, &p);
	if (*p != '#') {
		fprintf(stderr, "invalid entry: bad value: \"%s\"\n", in);
		exit(1);
	}

	/* color */
	p++;
	for(i=0; i<6; i++) {
		if ( ! (
		          (*p >= 'a' && *p <= 'f') ||
		          (*p >= 'A' && *p <= 'F') ||
		          (*p >= '0' && *p <= '9')
		       )
		) {
			fprintf(stderr, "invalid entry: bad color: \"%s\"\n", in);
			exit(1);
		}
		color[i] = *p;
		p++;
	}
	if (*p != ':') {
		fprintf(stderr, "invalid entry: bad format: \"%s\"\n", in);
		exit(1);
	}
	color[6] = '\0';

	/* extrude */
	p++;
	if (*p != ':') {
		extrude = strtod(p, &p);
		if (*p != ':') {
			fprintf(stderr, "invalid entry: bad value: \"%s\"\n", in);
			exit(1);
		}
	}

	/* comment */
	p++;
	name = p;

	/* add */
	pie_add(co, value, color, extrude, name);
}

#define DLEN 128

static inline void load_data(char *fn, struct conf *co)
{
	FILE *f;
	char b[DLEN];
	char *start;
	char *p;

	/* if stdin */
	if (strcmp(fn, "-") == 0)
		f = stdin;

	/* open file */
	else {
		f = fopen(fn, "r");
		if (f == NULL) {
			fprintf(stderr, "can't open input data file\n");
			exit(1);
		}
	}

	while (1) {

		if (feof(f))
			break;;

		/* read line */
		memset(b, 0, DLEN);
		fgets(b, DLEN, f);

		/* strip start spaces */
		start = b;
		while (*start == ' ' || *start == '\t')
			start++;

		/* remove comment */
		if (*start == '#')
			continue;

		/* strip end spaces */
		p = start;
		while (*p != '\0')
			p++;
		while (p >= start && ( *p == '\0' || *p == '\n' || *p == '\t' || *p == '\r' || *p == ' ' ) )
			p--;
		p++;
		*p = '\0';

		/* if empty */
		if (*start == '\0')
			continue;

		/* load data */
		add_data(start, co);
	}

	fclose(f);
}

int main(int argc, char *argv[])
{
	struct conf *co;
	int nb;
	char *f_in = NULL;
	double ratio;
	int mode;
	char *file_out;
	int img_w;
	int img_h;

	co = pie_new();
	if (co == NULL) {
		fprintf(stderr, "Memory error\n");
		exit(1);
	}

	if (argc == 1)
		usage();

	/* read command line */
	for (nb=1; nb<argc; nb++) {

		/* on a fini de lire les options */
		if (argv[nb][0] != '-')
			goto parsing_end;

		switch (argv[nb][1]) {

		/* fini */
		case '-':
			nb ++;
			goto parsing_end;
			break;

		/* background color */
		case 'b':
			get_one(&nb, argc);
			pie_set_do_back(co, 1);
			pie_set_back_color(co, argv[nb]);
			break;

		/* line width */
		case 'c':
			get_one(&nb, argc);
			pie_set_line_width(co, atof(argv[nb]));
			break;

		/* line color */
		case 'C':
			get_one(&nb, argc);
			pie_set_line_color(co, argv[nb]);
			break;

		/* decal */
		case 'd':
			get_one(&nb, argc);
			pie_set_decal(co, atof(argv[nb]));
			break;

		/* decal */
		case 'e':
			get_one(&nb, argc);
			pie_set_height(co, atof(argv[nb]));
			break;

		/* output format */
		case 'f':
			get_one(&nb, argc);
			/**/ if (strcmp(argv[nb], "PNG") == 0)
				mode = 1;
			else if (strcmp(argv[nb], "EPS") == 0)
				mode = 2;
			else if (strcmp(argv[nb], "SVG") == 0)
				mode = 3;
			else if (strcmp(argv[nb], "PDF") == 0)
				mode = 4;
			else {
				fprintf(stderr, "unknown format %s\n", argv[nb]);
				exit(1);
			}
			break;

		/* height */
		case 'h':
			get_one(&nb, argc);
			img_h = atoi(argv[nb]);
			pie_set_img_h(co, img_h);
			break;

		/* input file */
		case 'i':
			get_one(&nb, argc);
			f_in = argv[nb];
			break;

		/* legend color */
		case 'l':
			get_one(&nb, argc);
			pie_set_do_legend(co, 1);
			pie_set_legend_color(co, argv[nb]);
			break;

		/* legend size */
		case 'L':
			get_one(&nb, argc);
			pie_set_do_legend(co, 1);
			pie_set_legend_size(co, atof(argv[nb]));
			break;

		/* margin */
		case 'm':
			get_one(&nb, argc);
			pie_set_margin(co, atof(argv[nb]));
			break;

		/* output */
		case 'o':
			get_one(&nb, argc);
			file_out = argv[nb];
			break;

		/* ratio */
		case 'r':
			get_one(&nb, argc);
			ratio = atof(argv[nb]);
			if (ratio < 0 || ratio > 1) {
				fprintf(stderr, "ratio must be >= 0 and <= 1\n");
				exit(1);
			}
			pie_set_ratio(co, ratio);
			break;

		/* title size */
		case 's':
			get_one(&nb, argc);
			pie_set_title_size(co, atoi(argv[nb]));
			break;

		/* title */
		case 't':
			get_one(&nb, argc);
			pie_set_title(co, argv[nb]);
			break;

		/* title color */
		case 'T':
			get_one(&nb, argc);
			pie_set_title_color(co, argv[nb]);
			break;

		/* width */
		case 'w':
			get_one(&nb, argc);
			img_w = atoi(argv[nb]);
			pie_set_img_w(co, img_w);
			break;
		}
	}

parsing_end:

	/* check */
	if (file_out == NULL) {
		fprintf(stderr, "output name is mandatory\n");
		exit(1);
	}

	/* load data */
	for (; nb<argc; nb++)
		add_data(argv[nb], co);

	/* load data from file */
	if (f_in != NULL)
		load_data(f_in, co);

	/* create image */
	pie_draw(co, mode, file_out);

	return 0;
}
