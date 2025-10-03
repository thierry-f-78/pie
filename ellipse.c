/*
 * Copyright 2010-2019 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 */
#include <math.h>
#include <cairo.h>

/*
 * x = h + a * cos(t);
 * y = k + b * sin(t);
 *
 */
void cairo_ellipse(cairo_t *c, double x, double y, double a, double b, double start, double stop)
{
	double i;

	while (stop < start)
		stop += 2.0f * M_PI;

	cairo_line_to(c, x+(a*cos(start)), y+(b*sin(start)));
	for (i=start; i<stop; i += 0.01f)
		cairo_line_to(c, x+(a*cos(i)), y+(b*sin(i)));
	cairo_line_to(c, x+(a*cos(stop)), y+(b*sin(stop)));
}

void cairo_ellipse_negative(cairo_t *c, double x, double y, double a, double b, double start, double stop)
{
	double i;

	while (stop > start)
		stop -= 2.0f * M_PI;

	cairo_move_to(c, x+(a*cos(start)), y+(b*sin(start)));
	for (i=start; i>stop; i -= 0.01f)
		cairo_line_to(c, x+(a*cos(i)), y+(b*sin(i)));
	cairo_line_to(c, x+(a*cos(stop)), y+(b*sin(stop)));
}

#if 0

int ____main(void) {
	cairo_t *c;
	cairo_surface_t *s;

	s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 300, 300);
	c = cairo_create(s);

	cairo_set_line_width(c, 1.0f);

	cairo_move_to(c, 150, 150);
	cairo_ellipse_negative(c, 150, 150, 100, 50, 0, M_PI/2);
	cairo_line_to(c, 150, 150);
	cairo_fill(c);
	cairo_stroke(c);

	cairo_move_to(c, 160, 160);
	cairo_ellipse(c, 160, 160, 100, 50, 0, M_PI/2);
	cairo_line_to(c, 160, 160);
	cairo_fill(c);
	cairo_stroke(c);



	cairo_surface_write_to_png(s, "z.png");

	return 0;
}

#endif
