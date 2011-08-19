pkglib_LTLIBRARIES = wholesale.la
wholesale_la_SOURCES = init.cpp main.cpp wholesale.h \
	solver_matpower.cpp solver_matpower.h include/mclmcrrt.h include/mclcppclass.h\
	bus.cpp bus.h \
	line.cpp line.h \
	# NEWCLASS 
# DO NOT EDIT THE PREVIOUS LINE

wholesale_la_CPPFLAGS = -I/mnt/matlab/extern/include

# TODO set the version info as desired
wholesale_la_LDFLAGS = -L/mnt/kanderson/gridlab/wholesale -lopf -lmwmclmcrrt -lmwmclmcrrt -L/mnt/matlab/runtime/glnx86 -module -no-undefined -avoid-version -version-info 0:0:0

uninstall-hook:
	-rmdir $(DESTDIR)$(pkglibdir)

