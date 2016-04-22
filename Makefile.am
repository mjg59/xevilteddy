# GPL (C) 1998 Andreas Tille
#   tille@physik.uni-halle.de
bin_PROGRAMS = xteddy

xteddy_SOURCES = xteddy.c 
HTML_DOC = html/xteddy.html html/xteddy_info.html \
           html/images/noise.jpg html/images/xteddy.gif html/images/xteddy.xbm
xteddy_LDADD = -L/usr/X11R6/lib
LIBS = -lX11 -lXext -lXpm -lXtst
man_MANS = xteddy.1

pixmapdir = $(prefix)/include/X11
                                                                                                    
pixmap_DATA = pixmaps/xteddy_bw.xbm      \
              pixmaps/xteddy_color.xpm   \
              pixmaps/xteddy_icon.xbm    \
              pixmaps/xteddy_mask.xbm    \
              pixmaps/xpenguin_bw.xbm    \
              pixmaps/xpenguin_color.xpm \
              pixmaps/xpenguin_icon.xbm  \
              pixmaps/xpenguin_mask.xbm  \
	      pixmaps/teddy_bw.xbm       \
	      pixmaps/teddy_color.xpm    \
	      pixmaps/teddy_icon.xbm     \
	      pixmaps/teddy_mask.xbm

EXTRA_DIST = xteddy.README configure xteddy.1 autogen.sh \
             $(HTML_DOC) $(pixmap_DATA)
	  


