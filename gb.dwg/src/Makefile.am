COMPONENT = gb.dwg
include ../component.am

gblib_LTLIBRARIES = gb.dwg.la

gb_dwg_la_LIBADD = @LIBREDWG_LIBS@
gb_dwg_la_LDFLAGS = -no-undefined -module @LD_FLAGS@
gb_dwg_la_CFLAGS = @LIBREDWG_INC@ -Wno-discarded-qualifiers

EXTRA_HEADERS = main.h \
	c_dwg.h \
	c_ents.h

gb_dwg_la_SOURCES = \
	main.c \
	c_dwg.c \
	c_ents.c \
	$(EXTRA_HEADERS)

