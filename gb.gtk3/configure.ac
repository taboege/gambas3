dnl ---- configure.ac for gb.gtk3 component

m4_include([../version.m4])
AC_INIT(gambas3-gb-gtk3, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.gtk3)
AC_PROG_LIBTOOL

GB_CHECK_XWINDOW()

GB_COMPONENT_PKG_CONFIG(
  gtk3, GTK3, gb.gtk3, [src],
  'gtk+-3.0 >= 3.4' 'librsvg-2.0 >= 2.14.3' 'cairo >= 1.6.0' 'cairo-ft >= 1.6.0' 'gtk+-unix-print-3.0 >= 3.4' sm ice
)

GB_COMPONENT_PKG_CONFIG(
  gtk3x11, GTK3X11, gb.gtk3.x11, [x11],
  'gtk+-3.0 >= 3.4' gdk-x11-3.0
)

GB_COMPONENT_PKG_CONFIG(
  gtk3wayland, GTK3WAYLAND, gb.gtk3.wayland, [wayland],
  'gtk+-3.0 >= 3.4' gdk-wayland-3.0
)

GB_COMPONENT_PKG_CONFIG(
  gtk3webview, GTK3WEBVIEW, gb.gtk3.webview, [webview],
  'gtk+-3.0 >= 3.4' 'webkit2gtk-4.0 >= 2.20'
)

AC_OUTPUT( \
Makefile \
src/Makefile \
src/x11/Makefile \
src/wayland/Makefile \
src/webview/Makefile
)

GB_PRINT_MESSAGES
