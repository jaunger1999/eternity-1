// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2000 James Haley
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//--------------------------------------------------------------------------
//
// DESCRIPTION:
//  Gamma correction LUT stuff.
//  Color range translation support
//  Functions to draw patches (by post) directly to screen.
//  Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

#include "z_zone.h"
#include "i_system.h"

#include "c_io.h"
#include "d_gi.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_video.h"
#include "m_bbox.h"
#include "r_draw.h"
#include "r_main.h"
#include "v_patch.h" // haleyjd
#include "v_misc.h"
#include "v_patchfmt.h"
#include "v_video.h"
#include "w_wad.h"   /* needed for color translation lump lookup */


// Each screen is [SCREENWIDTH*SCREENHEIGHT];
// SoM: Moved. See cb_video_t
// byte *screens[5];
int  dirtybox[4];

//jff 2/18/98 palette color ranges for translation
//jff 4/24/98 now pointers set to predefined lumps to allow overloading

byte *cr_brick;
byte *cr_tan;
byte *cr_gray;
byte *cr_green;
byte *cr_brown;
byte *cr_gold;
byte *cr_red;
byte *cr_blue;
byte *cr_blue_status;
byte *cr_orange;
byte *cr_yellow;

//jff 4/24/98 initialize this at runtime
byte *colrngs[10];

// Now where did these came from?
byte gammatable[5][256] =
{
  {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
   17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
   33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
   49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
   65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
   81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
   97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
   113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255},

  {2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,24,25,26,27,29,30,31,
   32,33,34,36,37,38,39,40,41,42,44,45,46,47,48,49,50,51,52,54,55,
   56,57,58,59,60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,77,
   78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,
   99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
   115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,129,
   130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,
   146,147,148,148,149,150,151,152,153,154,155,156,157,158,159,160,
   161,162,163,163,164,165,166,167,168,169,170,171,172,173,174,175,
   175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,189,
   190,191,192,193,194,195,196,196,197,198,199,200,201,202,203,204,
   205,205,206,207,208,209,210,211,212,213,214,214,215,216,217,218,
   219,220,221,222,222,223,224,225,226,227,228,229,230,230,231,232,
   233,234,235,236,237,237,238,239,240,241,242,243,244,245,245,246,
   247,248,249,250,251,252,252,253,254,255},

  {4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,33,35,36,38,39,40,42,
   43,45,46,47,48,50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,69,
   70,72,73,74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,91,92,93,
   94,95,96,97,98,100,101,102,103,104,105,106,107,108,109,110,111,112,
   113,114,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
   129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,144,
   144,145,146,147,148,149,150,151,152,153,153,154,155,156,157,158,159,
   160,160,161,162,163,164,165,166,166,167,168,169,170,171,172,172,173,
   174,175,176,177,178,178,179,180,181,182,183,183,184,185,186,187,188,
   188,189,190,191,192,193,193,194,195,196,197,197,198,199,200,201,201,
   202,203,204,205,206,206,207,208,209,210,210,211,212,213,213,214,215,
   216,217,217,218,219,220,221,221,222,223,224,224,225,226,227,228,228,
   229,230,231,231,232,233,234,235,235,236,237,238,238,239,240,241,241,
   242,243,244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,
   255},

  {8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,47,49,50,52,53,55,
   57,58,60,61,63,64,65,67,68,70,71,72,74,75,76,77,79,80,81,82,84,85,
   86,87,88,90,91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
   108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
   125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,139,140,
   141,142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,
   155,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,
   169,170,171,172,173,173,174,175,176,176,177,178,179,180,180,181,182,
   183,183,184,185,186,186,187,188,189,189,190,191,192,192,193,194,195,
   195,196,197,197,198,199,200,200,201,202,202,203,204,205,205,206,207,
   207,208,209,210,210,211,212,212,213,214,214,215,216,216,217,218,219,
   219,220,221,221,222,223,223,224,225,225,226,227,227,228,229,229,230,
   231,231,232,233,233,234,235,235,236,237,237,238,238,239,240,240,241,
   242,242,243,244,244,245,246,246,247,247,248,249,249,250,251,251,252,
   253,253,254,254,255},

  {16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,66,68,69,71,73,75,76,
   78,80,81,83,84,86,87,89,90,92,93,94,96,97,98,100,101,102,103,105,106,
   107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,
   125,126,128,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
   142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,
   156,157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
   169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,181,
   182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,192,193,
   193,194,195,195,196,196,197,198,198,199,200,200,201,202,202,203,203,
   204,205,205,206,207,207,208,208,209,210,210,211,211,212,213,213,214,
   214,215,216,216,217,217,218,219,219,220,220,221,221,222,223,223,224,
   224,225,225,226,227,227,228,228,229,229,230,230,231,232,232,233,233,
   234,234,235,235,236,236,237,237,238,239,239,240,240,241,241,242,242,
   243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,
   251,252,252,253,254,254,255,255}
};

int usegamma;

//
// V_InitColorTranslation
//
// Loads the color translation tables from predefined lumps at game start
// No return value
//
// Used for translating text colors from the red palette range
// to other colors. The first nine entries can be used to dynamically
// switch the output of text color thru the HUlib_drawText routine
// by embedding ESCn in the text to obtain color n. Symbols for n are
// provided in v_video.h.
//

typedef struct crdef_s
{
  const char *name;
  byte **map1, **map2;
} crdef_t;

// killough 5/2/98: table-driven approach
static const crdef_t crdefs[] = 
{
   { "CRBRICK",  &cr_brick,   &colrngs[CR_BRICK ] },
   { "CRTAN",    &cr_tan,     &colrngs[CR_TAN   ] },
   { "CRGRAY",   &cr_gray,    &colrngs[CR_GRAY  ] },
   { "CRGREEN",  &cr_green,   &colrngs[CR_GREEN ] },
   { "CRBROWN",  &cr_brown,   &colrngs[CR_BROWN ] },
   { "CRGOLD",   &cr_gold,    &colrngs[CR_GOLD  ] },
   { "CRRED",    &cr_red,     &colrngs[CR_RED   ] },
   { "CRBLUE",   &cr_blue,    &colrngs[CR_BLUE  ] },
   { "CRORANGE", &cr_orange,  &colrngs[CR_ORANGE] },
   { "CRYELLOW", &cr_yellow,  &colrngs[CR_YELLOW] },
   { "CRBLUE2",  &cr_blue_status, &cr_blue_status },
   { NULL }
};

// killough 5/2/98: tiny engine driven by table above
void V_InitColorTranslation(void)
{
  register const crdef_t *p;
  for (p=crdefs; p->name; p++)
    *p->map1 = *p->map2 = (byte *)(wGlobalDir.CacheLumpName(p->name, PU_STATIC));
}

//
// vrect_t
//
// haleyjd 05/04/10: clippable and scalable rect structure.
// TODO: make available to v_block.c
//
typedef struct vrect_s
{
   int x;   // original x coordinate for upper left corner
   int y;   // original y coordinate for upper left corner
   int w;   // original width
   int h;   // original height

   int cx1; // clipped x coordinate for left edge
   int cx2; // clipped x coordinate for right edge
   int cy1; // clipped y coordinate for upper edge
   int cy2; // clipped y coordinate for lower edge
   int cw;  // clipped width
   int ch;  // clipped height

   int sx;  // scaled x
   int sy;  // scaled y
   int sw;  // scaled width
   int sh;  // scaled height
} vrect_t;

//
// V_clipRect
//
// haleyjd 05/04/10: OO rect clipping.
// TODO: use this in v_block.c, which does identical logic.
//
static void V_clipRect(vrect_t *r, VBuffer *buffer)
{
   // clip to left and top edges
   r->cx1 = r->x >= 0 ? r->x : 0;
   r->cy1 = r->y >= 0 ? r->y : 0;

   // determine right and bottom edges
   r->cx2 = r->x + r->w - 1;
   r->cy2 = r->y + r->h - 1;

   // clip right and bottom edges
   if(r->cx2 >= buffer->unscaledw)
      r->cx2 = buffer->unscaledw - 1;
   if(r->cy2 >= buffer->unscaledh)
      r->cy2 = buffer->unscaledh - 1;

   // determine clipped width and height
   r->cw = r->cx2 - r->cx1 + 1;
   r->ch = r->cy2 - r->cy1 + 1;
}

//
// V_scaleRect
//
// haleyjd 05/04/10: OO rect scaling.
// TODO: use this in v_block.c, which does identical logic.
//
static void V_scaleRect(vrect_t *r, VBuffer *buffer)
{
   r->sx = buffer->x1lookup[r->cx1];
   r->sy = buffer->y1lookup[r->cy1];
   r->sw = buffer->x2lookup[r->cx2] - r->sx + 1;
   r->sh = buffer->y2lookup[r->cy2] - r->sy + 1;

#ifdef RANGECHECK
   // sanity check - out-of-bounds values should not come out of the scaling
   // arrays so long as they are accessed within bounds.
   if(r->sx < 0 || r->sx + r->sw > buffer->width ||
      r->sy < 0 || r->sy + r->sh > buffer->height)
   {
      I_Error("V_scaleRect: internal error - invalid scaling lookups\n");
   }
#endif
}

//
// V_CopyRect
//
// Copies a source rectangle in a screen buffer to a destination
// rectangle in another screen buffer. Source origin in srcx,srcy,
// destination origin in destx,desty, common size in width and height.
// Source buffer specfified by srcscrn, destination buffer by destscrn.
//
// Marks the destination rectangle on the screen dirty.
//
// No return value.
//
// haleyjd 05/04/10: rewritten definitively.
//
void V_CopyRect(int srcx, int srcy, VBuffer *src, int width,
		          int height, int destx, int desty, VBuffer *dest)
{
   vrect_t srcrect, dstrect;
   int usew, useh;
   byte *srcp, *dstp;

#ifdef RANGECHECK
   // SoM: Do not attempt to copy across scaled buffers with different scales
   if(src->scaled  != dest->scaled  || 
      src->ixscale != dest->ixscale || 
      src->iyscale != dest->iyscale)
   {
      I_Error("V_CopyRect: src and dest VBuffers have different scaling\n");
   }
#endif

   // quick rejection if source rect is off-screen
   if(srcx + width < 0 || srcy + height < 0 || 
      srcx >= src->unscaledw || srcy >= src->unscaledh)
      return;

   // quick rejection if dest rect is off-screen
   if(destx + width < 0 || desty + height < 0 ||
      destx >= dest->unscaledw || desty >= dest->unscaledh)
      return;

   // populate source rect
   srcrect.x = srcx;
   srcrect.y = srcy;
   srcrect.w = width;
   srcrect.h = height;

   // clip source rect on source surface
   V_clipRect(&srcrect, src);

   // clipped away completely?
   if(srcrect.cw <= 0 || srcrect.ch <= 0)
      return;

   // populate dest rect
   dstrect.x = destx;
   dstrect.y = desty;
   dstrect.w = width;
   dstrect.h = height;

   // clip dest rect on dest surface
   V_clipRect(&dstrect, dest);

   // clipped away completely?
   if(dstrect.cw <= 0 || dstrect.ch <= 0)
      return;

   // scale rects?
   if(src->scaled)
   {
      V_scaleRect(&srcrect, src);
      V_scaleRect(&dstrect, dest);

      srcp = src->ylut[srcrect.sy]  + src->xlut[srcrect.sx];
      dstp = dest->ylut[dstrect.sy] + dest->xlut[dstrect.sx];

	  // use the smaller of the two scaled rect widths / heights
	  usew = (srcrect.sw < dstrect.sw ? srcrect.sw : dstrect.sw);
	  useh = (srcrect.sh < dstrect.sh ? srcrect.sh : dstrect.sh);

   }
   else
   {
      srcp = src->ylut[srcrect.cy1]  + src->xlut[srcrect.cx1];
      dstp = dest->ylut[dstrect.cy1] + dest->xlut[dstrect.cx1];

	  // use the smaller of the two clipped rect widths / heights
      usew = (srcrect.cw < dstrect.cw ? srcrect.cw : dstrect.cw);
      useh = (srcrect.ch < srcrect.ch ? srcrect.ch : dstrect.ch);
   }

   // block copy
   if(src->pitch == dest->pitch && usew == src->width && usew == dest->width)
   {
      memcpy(dstp, srcp, src->pitch * useh);
   }
   else
   {
      while(useh--)
      {
         memcpy(dstp, srcp, usew);
         srcp += src->pitch;
         dstp += dest->pitch;
      }
   }
}

//
// V_DrawPatch
//
// Masks a column based masked pic to the screen.
//
// The patch is drawn at x,y in the buffer selected by scrn
// No return value
//
// V_DrawPatchFlipped
//
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
// Patch is drawn at x,y in screenbuffer scrn.
// No return value
//
// killough 11/98: Consolidated V_DrawPatch and V_DrawPatchFlipped into one
// haleyjd  04/04: rewritten to use new ANYRES patch system
//
void V_DrawPatchGeneral(int x, int y, VBuffer *buffer, patch_t *patch,
			            bool flipped)
{
   PatchInfo pi;

   pi.x = x;
   pi.y = y;
   pi.patch = patch;
   pi.flipped = flipped;
   pi.drawstyle = PSTYLE_NORMAL;

   V_DrawPatchInt(&pi, buffer);
}

//
// V_DrawPatchTranslated
//
// Masks a column based masked pic to the screen.
// Also translates colors from one palette range to another using
// the color translation lumps loaded in V_InitColorTranslation
//
// The patch is drawn at x,y in the screen buffer scrn. Color translation
// is performed thru the table pointed to by outr. cm is not used.
//
// jff 1/15/98 new routine to translate patch colors
// haleyjd 04/03/04: rewritten for ANYRES patch system
//
void V_DrawPatchTranslated(int x, int y, VBuffer *buffer, patch_t *patch,
                           byte *outr, bool flipped)
{
   PatchInfo pi;
   
   pi.x = x;
   pi.y = y;
   pi.patch = patch;
   pi.flipped = flipped;

   // is the patch really translated?
   if(outr)
   {
      pi.drawstyle = PSTYLE_TLATED;   
      V_SetPatchColrng(outr);
   }
   else
      pi.drawstyle = PSTYLE_NORMAL;

   V_DrawPatchInt(&pi, buffer);
}

//
// V_DrawPatchTranslatedLit
//
// haleyjd 01/22/12: Translated patch drawer that also supports a secondary
// lighting remapping.
//
void V_DrawPatchTranslatedLit(int x, int y, VBuffer *buffer, patch_t *patch,
                              byte *outr, byte *lighttable, bool flipped)
{
   PatchInfo pi;
   
   pi.x = x;
   pi.y = y;
   pi.patch = patch;
   pi.flipped = flipped;

   // is the patch really translated?
   if(outr)
   {
      if(lighttable) // is it really lit?
      {
         pi.drawstyle = PSTYLE_TLATEDLIT;
         V_SetPatchLight(lighttable);
      }
      else
      {
         pi.drawstyle = PSTYLE_TLATED;   
      }
      V_SetPatchColrng(outr);
   }
   else if(lighttable)
   {
      // still treat as translated; just use the lighttable
      pi.drawstyle = PSTYLE_TLATED;
      V_SetPatchColrng(lighttable);
   }
   else
      pi.drawstyle = PSTYLE_NORMAL;

   V_DrawPatchInt(&pi, buffer);
}



//
// V_DrawPatchTL
//
// Masks a column based masked pic to the screen with translucency.
// Also translates colors from one palette range to another using
// the color translation lumps loaded in V_InitColorTranslation.
//
// haleyjd 04/03/04: rewritten for ANYRES patch system
// 
void V_DrawPatchTL(int x, int y, VBuffer *buffer, patch_t *patch,
                   byte *outr, int tl)
{
   PatchInfo pi;

   // is invisible?
   if(tl == 0)
      return;

   // if translucency is off or is opaque, fall back to translated
   if(!general_translucency || tl == FRACUNIT)
   {
      V_DrawPatchTranslated(x, y, buffer, patch, outr, false);
      return;
   }

   pi.x = x;
   pi.y = y;
   pi.patch = patch;
   pi.flipped = false; // TODO: these could be flipped too now

   // is the patch translated as well as translucent?
   if(outr)
   {
      pi.drawstyle = PSTYLE_TLTRANSLUC;
      V_SetPatchColrng(outr);
   }
   else
      pi.drawstyle = PSTYLE_TRANSLUC;

   // figure out the RGB tables to use for the tran level
   {
      unsigned int fglevel, bglevel;
      fglevel = tl & ~0x3ff;
      bglevel = FRACUNIT - fglevel;
      V_SetPatchTL(Col2RGB8[fglevel >> 10], Col2RGB8[bglevel >> 10]);
   }

   V_DrawPatchInt(&pi, buffer);
}

//
// V_DrawPatchAdd
//
// Masks a column based masked pic to the screen with additive
// translucency and optional color translation.
//
// haleyjd 02/08/05
// 
void V_DrawPatchAdd(int x, int y, VBuffer *buffer, patch_t *patch,
                    byte *outr, int tl)
{
   PatchInfo pi;

   // if translucency is off, fall back to translated
   if(!general_translucency)
   {
      V_DrawPatchTranslated(x, y, buffer, patch, outr, false);
      return;
   }

   pi.x = x;
   pi.y = y;
   pi.patch = patch;
   pi.flipped = false; // TODO: these could be flipped too now

   // is the patch translated as well as translucent?
   if(outr)
   {
      pi.drawstyle = PSTYLE_TLADD;
      V_SetPatchColrng(outr);
   }
   else
      pi.drawstyle = PSTYLE_ADD;

   // figure out the RGB tables to use for the tran level
   {
      unsigned int fglevel, bglevel;
      fglevel = tl & ~0x3ff;    // normal foreground level
      bglevel = FRACUNIT;       // full background level
      V_SetPatchTL(Col2RGB8_LessPrecision[fglevel >> 10], 
                   Col2RGB8_LessPrecision[bglevel >> 10]);
   }

   V_DrawPatchInt(&pi, buffer);
}

// blackmap is used by V_DrawPatchShadowed; see below.
static byte blackmap[256];

//
// V_BuildBlackMap
//
// Builds an all-black colormap using GameModeInfo->blackIndex, to provide a
// map useful for patch shadowing. This is much more reliable than using level
// 33 of colormap 0, into which some wads seem to place some data which could
// be called interesting, to say the least.
//
static void V_BuildBlackMap(void)
{
   memset(blackmap, GameModeInfo->blackIndex, 256);
}

//
// V_DrawPatchShadowed
//
// Convenience routine; draws the patch twice, first using a black colormap,
// and then normally.
//
void V_DrawPatchShadowed(int x, int y, VBuffer *buffer, patch_t *patch,
                         byte *outr, int tl)
{
   static bool firsttime = true;

   if(firsttime)
   {
      V_BuildBlackMap();
      firsttime = false;
   }

   V_DrawPatchTL(x + 2, y + 2, buffer, patch, blackmap, FRACUNIT*2/3);
   V_DrawPatchTL(x,     y,     buffer, patch, outr,     tl);
}

//
// V_DrawBlock
//
// Draw a linear block of pixels into the view buffer. 
//
// The bytes at src are copied in linear order to the screen rectangle
// at x,y in screenbuffer scrn, with size width by height.
//
// No return value.
//
// haleyjd 04/08/03: rewritten for ANYRES system -- see v_block.c
// 
void V_DrawBlock(int x, int y, VBuffer *buffer, int width, int height, byte *src)
{
   buffer->BlockDrawer(x, y, buffer, width, height, src);
}

//
// V_DrawMaskedBlockTR
//
// Draw a translated, masked linear block of pixels into the view buffer. 
//
// haleyjd 06/29/08
// 
void V_DrawMaskedBlockTR(int x, int y, VBuffer *buffer, int width, int height,
                         int srcpitch, byte *src, byte *cmap)
{
   buffer->MaskedBlockDrawer(x, y, buffer, width, height, srcpitch, src, cmap);
}

//
// V_DrawBlockFS
//
// haleyjd 05/18/09: Convenience routine to do V_DrawBlock but with
// the assumption that the graphic is fullscreen, 320x200.
//
void V_DrawBlockFS(VBuffer *buffer, byte *src)
{
   buffer->BlockDrawer(0, 0, buffer, SCREENWIDTH, SCREENHEIGHT, src);
}

//
// V_DrawPatchFS
//
// haleyjd 05/18/09: Convenience routine to do V_DrawPatch but with
// the assumption that the graphic is fullscreen, 320x200.
//
void V_DrawPatchFS(VBuffer *buffer, patch_t *patch)
{
   V_DrawPatchGeneral(0, 0, buffer, patch, false);
}

//
// V_DrawFSBackground
//
// haleyjd 05/18/09: A single smart function which will determine the
// format of a fullscreen graphic resource and draw it properly.
//
void V_DrawFSBackground(VBuffer *dest, int lumpnum)
{
   void *source;
   patch_t *patch;
   int len = wGlobalDir.LumpLength(lumpnum);

   switch(len)
   {
   case 4096:  // 64x64 flat
      source = wGlobalDir.CacheLumpNum(lumpnum, PU_CACHE);
      V_DrawBackgroundCached((byte *)source, dest);
      break;
   case 64000: // 320x200 linear
      source = wGlobalDir.CacheLumpNum(lumpnum, PU_CACHE);
      V_DrawBlockFS(dest, (byte *)source);
      break;
   default:    // anything else is treated like a patch (let god sort it out)
      patch = PatchLoader::CacheNum(wGlobalDir, lumpnum, PU_CACHE);
      V_DrawPatchFS(dest, patch);
      break;
   }
}

#ifdef DJGPP
//
// V_GetBlock
//
// Gets a linear block of pixels from the view buffer.
//
// The pixels in the rectangle at x,y in screenbuffer scrn with size
// width by height are linearly packed into the buffer dest.
// No return value
//
void V_GetBlock(int x, int y, int scrn, int width, int height, byte *dest)
{
  byte *src;
  int p;

#ifdef RANGECHECK
  if (x<0
      ||x+width > SCREENWIDTH
      || y<0
      || y+height > SCREENHEIGHT
      || (unsigned int)scrn>4 )
    I_Error("Bad V_GetBlock\n");
#endif

   // SoM 1-30-04: ANYRES
   if(video.scaled)
   {
      width = video.x2lookup[x + width - 1] - video.x1lookup[x] + 1;
      x = video.x1lookup[x];

      height = video.y2lookup[y + height - 1] - video.y1lookup[y] + 1;
      y = video.y1lookup[y];
   }

   p = scrn == 0 ? video.pitch : video.width;

   src = video.screens[scrn] + y * p + x;
   while (height--)
   {
      memcpy (dest, src, width);
      src += p;
      dest += width;
   }
}
#endif

// 
// V_FindBestColor
//
// Adapted from zdoom -- thanks to Randy Heit.
//
// This always assumes a 256-color palette;
// it's intended for use in startup functions to match hard-coded
// color values to the best fit in the game's palette (allows
// cross-game usage among other things).
//
byte V_FindBestColor(const byte *palette, int r, int g, int b)
{
   int i, dr, dg, db;
   int bestcolor, bestdistortion, distortion;

   // use color 0 as a worst-case match for any color
   bestdistortion = 257*257*3;
   bestcolor = 0;

   for(i = 0; i < 256; i++)
   {
      dr = r - *palette++;
      dg = g - *palette++;
      db = b - *palette++;
      
      distortion = dr*dr + dg*dg + db*db;

      if(distortion < bestdistortion)
      {
         // exact match
         if(!distortion)
            return i;
         
         bestdistortion = distortion;
         bestcolor = i;
      }
   }

   return bestcolor;
}

// haleyjd: DOSDoom-style single translucency lookup-up table
// generation code. This code has a 32k (plus a bit more) 
// footprint but allows a much wider range of translucency effects
// than BOOM-style translucency. This will be used for particles, 
// for variable mapthing trans levels, and for screen patches.

// haleyjd: Updated 06/21/08 to use 32k lookup, mainly to fix
// additive translucency. Note this code is included in Odamex and
// so it can be considered GPL as used here, rather than BSD. But,
// I don't care either way. It is effectively dual-licensed I suppose.
// So, you can use it under this license if you wish:
//
// Copyright 1998-2012 Randy Heit  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

bool flexTranInit = false;
unsigned int  Col2RGB8[65][256];
unsigned int *Col2RGB8_LessPrecision[65];
byte RGB32k[32][32][32];

static unsigned int Col2RGB8_2[63][256];

#define MAKECOLOR(a) (((a)<<3)|((a)>>2))

typedef struct tpalcol_s
{
   unsigned int r, g, b;
} tpalcol_t;

void V_InitFlexTranTable(const byte *palette)
{
   int i, r, g, b, x, y;
   tpalcol_t  *tempRGBpal;
   const byte *palRover;

   // mark that we've initialized the flex tran table
   flexTranInit = true;
   
   tempRGBpal = (tpalcol_t *)(Z_Malloc(256*sizeof(*tempRGBpal), PU_STATIC, NULL));
   
   for(i = 0, palRover = palette; i < 256; i++, palRover += 3)
   {
      tempRGBpal[i].r = palRover[0];
      tempRGBpal[i].g = palRover[1];
      tempRGBpal[i].b = palRover[2];
   }

   // build RGB table
   for(r = 0; r < 32; ++r)
   {
      for(g = 0; g < 32; ++g)
      {
         for(b = 0; b < 32; ++b)
         {
            RGB32k[r][g][b] = 
               V_FindBestColor(palette, 
                               MAKECOLOR(r), MAKECOLOR(g), MAKECOLOR(b));
         }
      }
   }
   
   // build lookup table
   for(x = 0; x < 65; ++x)
   {
      for(y = 0; y < 256; ++y)
      {
         Col2RGB8[x][y] = (((tempRGBpal[y].r*x)>>4)<<20)  |
                            ((tempRGBpal[y].g*x)>>4) |
                          (((tempRGBpal[y].b*x)>>4)<<10);
      }
   }

   // build a secondary lookup with red and blue lsbs masked out for additive 
   // blending; otherwise, the overflow messes up the calculation and you get 
   // something very ugly.
   for(x = 1; x < 64; ++x)
   {
      Col2RGB8_LessPrecision[x] = Col2RGB8_2[x - 1];
      
      for(y = 0; y < 256; ++y)
         Col2RGB8_2[x-1][y] = Col2RGB8[x][y] & 0x3feffbff;
   }
   Col2RGB8_LessPrecision[0]  = Col2RGB8[0];
   Col2RGB8_LessPrecision[64] = Col2RGB8[64];

   Z_Free(tempRGBpal);
}

//
// V_CacheBlock
//
// haleyjd 12/22/02: 
// Copies a linear block to a memory buffer as if to a 
// low-res screen
//
void V_CacheBlock(int x, int y, int width, int height, byte *src,
                  byte *bdest)
{
   byte *dest = bdest + y*SCREENWIDTH + x;
   
   while(height--)
   {
      memcpy(dest, src, width);
      src += width;
      dest += SCREENWIDTH;
   }
}

//----------------------------------------------------------------------------
//
// $Log: v_video.c,v $
// Revision 1.10  1998/05/06  11:12:48  jim
// Formattted v_video.*
//
// Revision 1.9  1998/05/03  22:53:16  killough
// beautification, simplify translation lookup
//
// Revision 1.8  1998/04/24  08:09:39  jim
// Make text translate tables lumps
//
// Revision 1.7  1998/03/02  11:41:58  killough
// Add cr_blue_status for blue statusbar numbers
//
// Revision 1.6  1998/02/24  01:40:12  jim
// Tuned HUD font
//
// Revision 1.5  1998/02/23  04:58:17  killough
// Fix performance problems
//
// Revision 1.4  1998/02/19  16:55:00  jim
// Optimized HUD and made more configurable
//
// Revision 1.3  1998/02/17  23:00:36  jim
// Added color translation machinery and data
//
// Revision 1.2  1998/01/26  19:25:08  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:05  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------

