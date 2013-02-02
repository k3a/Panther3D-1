#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "prof.h"
#include "prof_internal.h"

#include "common.h"
#include "IP3DDraw2D.h"
#include "IP3DFont.h"

#pragma warning(disable:4305; disable:4244)

static IP3DFont* g_pFont=NULL;
static float line_spacing=10.0f;

// helper OpenGL -> Panther3D color
inline unsigned long glColor4f(float red, float green, float blue, float alpha)
{
	return P3DCOLOR_ARGB((int)(alpha*255), (int)(red*255), (int)(green*255), (int)(blue*255));
}
inline unsigned long glColor3ub(byte red, byte green, byte blue)
{
	return P3DCOLOR_XRGB(red, green, blue);
}
inline unsigned long glColor3fv(float *v)
{
	return P3DCOLOR_XRGB((int)(v[0]*255), (int)(v[1]*255), (int)(v[2]*255));
}

// use factor to compute a glow amount
static int get_colors(float factor,
                       float text_color_ret[3],
                       float glow_color_ret[3],
                       float *glow_alpha_ret)
{
   const float GLOW_RANGE = 0.5f;
   const float GLOW_ALPHA_MAX = 0.5f;
   float glow_alpha;
   int i;
   float hot[3] = {1, 1.0, 0.9};
   float cold[3] = {0.15, 0.9, 0.15};

   float glow_cold[3] = {0.5f, 0.5f, 0};
   float glow_hot[3] = {1.0f, 1.0f, 0};

   if (factor < 0) factor = 0;
   if (factor > 1) factor = 1;

   for (i=0; i < 3; ++i)
      text_color_ret[i] = cold[i] + (hot[i] - cold[i]) * factor;

   // Figure out whether to start up the glow as well.
   glow_alpha = (factor - GLOW_RANGE) / (1 - GLOW_RANGE);
   if (glow_alpha < 0) {
      *glow_alpha_ret = 0;
      return 0;
   }

   for (i=0; i < 3; ++i)
      glow_color_ret[i] = glow_cold[i] + (glow_hot[i] - glow_cold[i]) * factor;

   *glow_alpha_ret = glow_alpha * GLOW_ALPHA_MAX;
   return 1;
}

static void draw_rectangle(float x0, float y0, float x1, float y1, unsigned long col)
{
   // FACE_CULL is disabled so winding doesn't matter
   /*glVertex2f(x0, y0);
   glVertex2f(x1, y0);
   glVertex2f(x1, y1);
   glVertex2f(x0, y1);*/
   //return;
   g_p2D->Line_DrawFilledRect((int)x0, (int)y0, (int)(x1-x0), (int)(y0-y1), col);
}

static void printText(float x, float y, const char* txt, unsigned long col)
{
	if(!g_pFont) 
	{
		g_pFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
		g_pFont->CreateFont("Courier New", 16, true, false);
		line_spacing = g_pFont->GetLineH()+1.0f;
	}
	g_pFont->DrawText(FA_NOCLIP, txt, x, y, 0, 0, col);
}

float textWidth(const char* str)
{
	if(!g_pFont) 
	{
		g_pFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
		g_pFont->CreateFont("Courier New", 16, true, false);
		line_spacing = g_pFont->GetLineH()+1.0f;
	}
	return g_pFont->GetLineW(str);
}

typedef struct
{
   float x0,y0;
   float sx,sy;
} GraphLocation;

static float lastY=0;
static void graph_func(int id, int x0, int x1, float *values, void *data)
{
   GraphLocation *loc = (GraphLocation *) data;
   int i,r,g,b;
   unsigned long col;

   // trim out values that are under 0.2 ms to accelerate rendering
   while (x0 < x1 && (*values < 0.0002f)) { ++x0; ++values; }
   while (x1 > x0 && (values[x1-1-x0] < 0.0002f)) --x1;

   if (id == 0)
      col = glColor4f(1,1,1,0.5);
   else {
      if (x0 == x1) return;

      id = (id >> 8) + id;
      r = id * 37;
      g = id * 59;
      b = id * 45;
#pragma warning(disable:4761)
      col = glColor3ub((r & 127) + 80, (g & 127) + 80, (b & 127) + 80);
   }

   g_p2D->Line_Begin();
   
   if (x0 == x1) {
	   float x,y;
	   x = loc->x0 + x0 * loc->sx;
	   y = loc->y0 + values[0] * loc->sy;
	   //glVertex2f(x,loc->y0);
	   //glVertex2f(x, y);
	   g_p2D->Line_DrawSimple(x, loc->y0, x, y, col);
   }
   
   ///int maxI;
  /// if (x1-x0>10) maxI = x1-x0; else maxI = x1-x0-1;
   for (i=1; i < x1-x0; ++i) {
	   float x,y;
	   x = loc->x0 + (i+x0) * loc->sx;
	   y = g_pEngSet.Height/2.0f - values[i] * loc->sy;
	   
	   float xx,yy;
	   xx = loc->x0 + (i-1+x0) * loc->sx;
	   yy = g_pEngSet.Height/2.0f - values[i-1] * loc->sy;
	   //glVertex2f(x,y);
	  /// if (i==x1-x0) 
	  /// {
	  /// y = lastY;
	  /// }
	   g_p2D->Line_DrawSimple(x, y, xx, yy, col);
   }
 ///  lastY = g_pEngSet.Height/2.0f - values[0] * loc->sy;
   
  /* if (x1-x0==1)
   {
	   float x,y;
	   x = loc->x0 + (i+x0) * loc->sx;
	   y = g_pEngSet.Height/2.0f - values[i] * loc->sy;
	   g_p2D->Line_DrawSimple(x, y-1, x, y, col);
   }*/
   
   g_p2D->Line_End();
}

void Prof_draw_graph_dx(float sx, float sy, float x_spacing, float y_spacing)
{
#ifdef Prof_ENABLED
   Prof_Begin(iprof_draw_graph)
   GraphLocation loc = { sx, sy, x_spacing, y_spacing * 1000 };
   Prof_graph(128, graph_func, &loc);
   Prof_End
#endif
}


// float to string conversion with sprintf() was
// taking up 10-20% of the Prof_draw time, so I
// wrote a faster float-to-string converter

static char int_to_string[100][4];
static char int_to_string_decimal[100][4];
static char int_to_string_mid_decimal[100][4];
static void int_to_string_init(void)
{
   int i;
   for (i=0; i < 100; ++i) {
      sprintf(int_to_string[i], "%d", i);
      sprintf(int_to_string_decimal[i], ".%02d", i);
      sprintf(int_to_string_mid_decimal[i], "%d.%d", i/10, i % 10);
   }
}

static char *formats[5] =
{
   "%.0f",
   "%.1f",
   "%.2f",
   "%.3f",
   "%.4f",
};

static void float_to_string(char *buf, float num, int precision)
{
   int x,y;
   switch(precision) {
      case 2:
         if (num < 0 || num >= 100)
            break;
         x = num;
         y = (num - x) * 100;
         strcpy(buf, int_to_string[x]);
         strcat(buf, int_to_string_decimal[y]);
         return;
      case 3:
         if (num < 0 || num >= 10)
            break;
         num *= 10;
         x = num;
         y = (num - x) * 100;
         strcpy(buf, int_to_string_mid_decimal[x]);
         strcat(buf, int_to_string_decimal[y]+1);
         return;
      case 4:
         if (num < 0 || num >= 1)
            break;
         num *= 100;
         x = num;
         y = (num - x) * 100;
         buf[0] = '0';
         strcpy(buf+1, int_to_string_decimal[x]);
         strcat(buf, int_to_string_decimal[y]+1);
         return;
   }
   sprintf(buf, formats[precision], num);
}

void Prof_draw_dx(float sx, float sy, float full_width, float height, int precision)
{
#ifdef Prof_ENABLED
   Prof_Begin(iprof_draw)
   
   if(!g_p2D) return;

   int i,j,n,o;
   float backup_sy;

   float field_width = textWidth("5555.55");
   float name_width  = full_width - field_width * 3;
   float plus_width  = textWidth("+");
   unsigned long col;

   int max_records;

   Prof_Report *pob;

   if (!int_to_string[0][0]) int_to_string_init();

   if (precision < 1) precision = 1;
   if (precision > 4) precision = 4;

   pob = Prof_create_report();

   for (i=0; i < NUM_TITLE; ++i) {
      if (pob->title[i]) {
         float header_x0 = sx;
         float header_x1 = header_x0 + full_width;

         if (i == 0)
            col = glColor4f(0.1f, 0.3f, 0, 0.85);
         else
            col = glColor4f(0.2f, 0.1f, 0.1f, 0.85);

         g_p2D->Line_Begin();
         draw_rectangle(header_x0, sy-2, header_x1, sy-line_spacing-4, col);
         g_p2D->Line_End();

         if (i == 0) 
             col = glColor4f(0.7, 0.7, 0, 1);
         else
             col = glColor4f(0.8f, 0.1f, 0.1f, 1);

         printText(sx+2, sy, pob->title[i], col);

         sy += 1.5*line_spacing;
         height -= abs(line_spacing)*1.5;
      }
   }

   max_records = height / abs(line_spacing);

   o = 0;
   n = pob->num_record;
   if (n > max_records) n = max_records;
   if (pob->hilight >= o + n) {
      o = pob->hilight - n + 1;
   }

   backup_sy = sy;

   g_p2D->Line_Begin();
   col = glColor4f(0,0,0,0.85);
   draw_rectangle(sx, sy, sx + full_width, sy - line_spacing, col);
   sy += line_spacing;

   for (i = 0; i < n; i++) {
      float y0, y1;

      if (i & 1) {
         col = glColor4f(0.1, 0.1f, 0.2, 0.85);
      } else {
         col = glColor4f(0.1f, 0.1f, 0.3, 0.85);
      }
      if (i+o == pob->hilight)
         col = glColor4f(0.3f, 0.3f, 0.1f, 0.85);

      y0 = sy;
      y1 = sy - line_spacing;

      draw_rectangle(sx, y0, sx + full_width, y1, col);
      sy += line_spacing;
   }

   g_p2D->Line_End();
   sy = backup_sy;
   col = glColor4f(0.7,0.7,0.7,1);

   if (pob->header[0])
      printText(sx+8, sy, pob->header[0], col);

   for (j=1; j < NUM_HEADER; ++j)
      if (pob->header[j])
         printText(sx + name_width + field_width * (j-1) + 
            field_width/2 - textWidth(pob->header[j])/2, sy, pob->header[j], col);

   sy += line_spacing;

   for (i = 0; i < n; i++) {
      char buf[256], *b = buf;
      Prof_Report_Record *r = &pob->record[i+o];
      float text_color[3], glow_color[3];
      float glow_alpha;
      float x = sx + textWidth(" ") * r->indent + plus_width/2;
      if (r->prefix) {
         buf[0] = r->prefix;
         ++b;
      } else {
         x += plus_width;
      }
      if (r->number)
         sprintf(b, "%s (%d)", r->name, r->number);
      else
         sprintf(b, "%s", r->name);
      if (get_colors(r->heat, text_color, glow_color, &glow_alpha)) {
         col = glColor4f(glow_color[0], glow_color[1], glow_color[2], glow_alpha);
         printText(x+2, sy-1, buf, col);
      }
      col = glColor3fv(text_color);
      printText(x + 1, sy, buf, col);

      for (j=0; j < NUM_VALUES; ++j) {
         if (r->value_flag & (1 << j)) {
            int pad;
            float_to_string(buf, r->values[j], j == 2 ? 2 : precision);
            pad = field_width- plus_width - textWidth(buf);
            if (r->indent) pad += plus_width;
            printText(sx + pad + name_width + field_width * j, sy, buf, col);
         }
      }
              

      sy += line_spacing;
   }

   Prof_free_report(pob);

   Prof_End
#endif
}
