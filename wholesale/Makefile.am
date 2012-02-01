pkglib_LTLIBRARIES = wholesale.la
wholesale_la_SOURCES = init.cpp main.cpp wholesale.h \
	wholesale.cpp include/mclmcrrt.h include/mclcppclass.h\
	bus.cpp bus.h \
	line.cpp line.h \
	gen.cpp gen.h \
	gen_cost.cpp gen_cost.h \
	areas.cpp areas.h \
	baseMVA.cpp baseMVA.h \
	# NEWCLASS 
# DO NOT EDIT THE PREVIOUS LINE

wholesale_la_CPPFLAGS = -I/mnt/matlab/extern/include

# TODO set the version info as desired
wholesale_la_LDFLAGS = -L/mnt/yizheng/gridspice.simulator/wholesale -lopf -lmwmclmcrrt -lmwmclmcrrt -L/mnt/matlab/runtime/glnx86 -module -no-undefined -avoid-version -version-info 0:0:0

uninstall-hook:
	-rmdir $(DESTDIR)$(pkglibdir)

