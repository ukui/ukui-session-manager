// -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
/*
 * Copyright (C) 2015 Canonical Ltd
 *               2015, National University of Defense Technology(NUDT) & Kylin Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Mirco Müller <mirco.mueller@canonical.com
 *              Neil Jagdish Patel <neil.patel@canonical.com>
 */

#include "gsm-blur.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

double align (double val, gboolean odd)
{
        double fract = val - (int) val;

        if (odd) {
        // for strokes with an odd line-width
                if (fract != 0.5f)
                        return (double) ((int) val + 0.5f);
                else
                        return val;
        } else {
        // for strokes with an even line-width
                if (fract != 0.0f)
                        return (double) ((int) val);
                else
                        return val;
        }
}

void RoundedRect (cairo_t* cr,
                  double   aspect,
                  double   x,
                  double   y,
                  double   cornerRadius,
                  double   width,
                  double   height)
{
        // sanity check
        if (cairo_status(cr) != CAIRO_STATUS_SUCCESS &&
            cairo_surface_get_type(cairo_get_target(cr)) != CAIRO_SURFACE_TYPE_IMAGE)
                return;

        gboolean odd = cairo_get_line_width (cr) == 2.0 ? FALSE : TRUE;

        double radius = cornerRadius / aspect;

        // top-left, right of the corner
        cairo_move_to(cr, align (x + radius, odd), align (y, odd));

        // top-right, left of the corner
        cairo_line_to(cr, align(x + width - radius, odd), align(y, odd));

        // top-right, below the corner
        cairo_arc(cr,
                  align(x + width - radius, odd),
                  align(y + radius, odd),
                  radius,
                  -90.0f * G_PI / 180.0f,
                  0.0f * G_PI / 180.0f);

        // bottom-right, above the corner
        cairo_line_to(cr, align(x + width, odd), align(y + height - radius, odd));

        // bottom-right, left of the corner
        cairo_arc(cr,
                  align(x + width - radius, odd),
                  align(y + height - radius, odd),
                  radius,
                  0.0f * G_PI / 180.0f,
                  90.0f * G_PI / 180.0f);

        // bottom-left, right of the corner
        cairo_line_to(cr, align(x + radius, odd), align(y + height, odd));

        // bottom-left, above the corner
        cairo_arc(cr,
                  align(x + radius, odd),
                  align(y + height - radius, odd),
                  radius,
                  90.0f * G_PI / 180.0f,
                  180.0f * G_PI / 180.0f);

        // top-left, right of the corner
        cairo_arc(cr,
                  align(x + radius, odd),
                  align(y + radius, odd),
                  radius,
                  180.0f * G_PI / 180.0f,
                  270.0f * G_PI / 180.0f);
}

void blurinner (guchar* pixel,
                gint*   zR,
                gint*   zG,
                gint*   zB,
                gint*   zA,
                gint    alpha,
                gint    aprec,
                gint    zprec)
{
        gint   r;
        gint   g;
        gint   b;
        guchar a;

        r = *pixel;
        g = *(pixel + 1);
        b = *(pixel + 2);
        a = *(pixel + 3);

        *zR += (alpha * ((r << zprec) - *zR)) >> aprec;
        *zG += (alpha * ((g << zprec) - *zG)) >> aprec;
        *zB += (alpha * ((b << zprec) - *zB)) >> aprec;
        *zA += (alpha * ((a << zprec) - *zA)) >> aprec;

       *pixel       = *zR >> zprec;
       *(pixel + 1) = *zG >> zprec;
       *(pixel + 2) = *zB >> zprec;
       *(pixel + 3) = *zA >> zprec;
}

void blurrow (guchar* pixels,
              gint    width,
              gint    height,
              gint    channels,
              gint    line,
              gint    alpha,
              gint    aprec,
              gint    zprec)
{
        gint    zR;
        gint    zG;
        gint    zB;
        gint    zA;
        gint    index;
        guchar* scanline;

        scanline = &(pixels[line * width * channels]);

        zR = *scanline << zprec;
        zG = *(scanline + 1) << zprec;
        zB = *(scanline + 2) << zprec;
        zA = *(scanline + 3) << zprec;

        for (index = 0; index < width; index ++)
                blurinner (&scanline[index * channels], &zR, &zG, &zB, &zA, alpha, aprec, zprec);

        for (index = width - 2; index >= 0; index--)
                blurinner (&scanline[index * channels], &zR, &zG, &zB, &zA, alpha, aprec, zprec);
}

void blurcol (guchar* pixels,
              gint    width,
              gint    height,
              gint    channels,
              gint    x,
              gint    alpha,
              gint    aprec,
              gint    zprec)
{
        gint zR;
        gint zG;
        gint zB;
        gint zA;
        gint index;
        guchar* ptr;

        ptr = pixels;

        ptr += x * channels;

        zR = *((guchar*) ptr    ) << zprec;
        zG = *((guchar*) ptr + 1) << zprec;
        zB = *((guchar*) ptr + 2) << zprec;
        zA = *((guchar*) ptr + 3) << zprec;

        for (index = width; index < (height - 1) * width; index += width)
                blurinner ((guchar*) &ptr[index * channels], &zR, &zG, &zB, &zA, alpha, aprec, zprec);

        for (index = (height - 2) * width; index >= 0; index -= width)
                blurinner ((guchar*) &ptr[index * channels], &zR, &zG, &zB, &zA, alpha, aprec, zprec);
}

void expblur (guchar* pixels,
              gint     width,
              gint     height,
              gint     channels,
              gint     radius,
              gint     aprec,
              gint     zprec)
{
        gint alpha;
        gint row = 0;
        gint col = 0;

        if (radius < 1)
                return;

        // calculate the alpha such that 90% of
        // the kernel is within the radius.
        // (Kernel extends to infinity)
        alpha = (gint) ((1 << aprec) * (1.0f - expf (-2.3f / (radius + 1.f))));

        for (; row < height; row++)
                blurrow(pixels, width, height, channels, row, alpha, aprec, zprec);

        for(; col < width; col++)
                blurcol(pixels, width, height, channels, col, alpha, aprec, zprec);

        return;
}

void blur(cairo_surface_t* surface, int size)
{
//        if (cairo_status(cr) != CAIRO_STATUS_SUCCESS &&
//            cairo_surface_get_type(cairo_get_target(cr)) != CAIRO_SURFACE_TYPE_IMAGE)
//              return;

//      cairo_surface_t* surface;
        guchar*          pixels;
        guint            width;
        guint            height;
        cairo_format_t   format;

//      surface = cairo_get_target(cr);

        cairo_surface_flush(surface);

        pixels = cairo_image_surface_get_data(surface);
        format = cairo_image_surface_get_format(surface);
        width = cairo_image_surface_get_width (surface);
        height = cairo_image_surface_get_height (surface);

        switch (format)
        {
        case CAIRO_FORMAT_ARGB32:
                expblur (pixels, width, height, 4, size, 16, 7);
                break;
        case CAIRO_FORMAT_RGB24:
                expblur (pixels, width, height, 3, size, 16, 7);
                break;
        case CAIRO_FORMAT_A8:
                expblur (pixels, width, height, 1, size, 16, 7);
                break;
        default :
                // do nothing
                break;
        }

        // inform cairo we altered the surfaces contents
        cairo_surface_mark_dirty (surface);
}
