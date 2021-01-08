dnl ---- configure.ac for gb.dwg
dnl version.m4 needs to be symlinked here. see relink.sh
m4_include([version.m4])
AC_INIT(gambas3-gb-dwg, GB_VERSION, [rurban@cpan.org], [],
        [https://github.com/LibreDWG/gambas3-bindings])
AC_CONFIG_MACRO_DIR([m4])

GB_INIT_SHORT([gb.dwg])
GB_SYSTEM
GB_LIBTOOL
AC_PROG_CXX
AC_PROG_LIBTOOL

dnl if in libredwg srcdir skip that
if test -e ../dwg.i && test -d ../../bindings/gambas ; then
  have_libredwg=yes
  if test -e gambas.h ; then
    hdr=$(realpath gambas.h)
    #srcdir=$(dirname "$hdr")
    #exec_prefix="/usr/local"
  fi
  AC_DEFINE_UNQUOTED(LIBREDWG_VERSION_0_10_1, 1, LibreDWG version in-tree)
  AC_SUBST(LIBREDWG_LIBS, "-L$top_dwg/src -lredwg")
  AC_SUBST(LIBREDWG_INC, "-I$top_dwg/include")
else
  libredwg_minver=0.10.1
  GB_COMPONENT_SEARCH(
    libredwg, LIBREDWG, gb.dwg, [src],
    "libredwg >= $libredwg_minver",
    [GB_FIND(dwg.h, `pkg-config --variable=includedir libredwg`, libredwg)],
    [],
    [],
    []
  )
  if test "$have_libredwg" = "yes"; then
    pkg-config --atleast-version=$libredwg_minver libredwg
    AC_DEFINE_UNQUOTED(LIBREDWG_VERSION_0_10_1, $((1-$?)), LibreDWG version >= $libredwg_minver)
    AC_SUBST(LIBREDWG_LIBS, "`pkg-config "libredwg >= $libredwg_minver" --libs`")
    AC_SUBST(LIBREDWG_INC, "`pkg-config "libredwg >= $libredwg_minver" --cflags`")
  fi
fi

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
