pkglib_LTLIBRARIES = ev.la
ev_la_SOURCES = init.cpp main.cpp ev.h \
	fleet.cpp fleet.h \
	# NEWCLASS 
# DO NOT EDIT THE PREVIOUS LINE

# TODO set the version info as desired
ev_la_LDFLAGS = -module -no-undefined -avoid-version -version-info 0:0:0

uninstall-hook:
	-rmdir $(DESTDIR)$(pkglibdir)

