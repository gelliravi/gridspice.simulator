pkglib_LTLIBRARIES = smartmeter.la
smartmeter_la_SOURCES = init.cpp main.cpp smartmeter.h \
	direct_data.cpp direct_data.h db_access.cpp db_access.h \
	# NEWCLASS 
# DO NOT EDIT THE PREVIOUS LINE

# TODO set the version info as desired
smartmeter_la_LDFLAGS = -module -no-undefined -avoid-version -version-info 0:0:0

CXXFLAGS := -I/usr/include/mysql -I/usr/include/mysql++

LDFLAGS := -L/usr/local/lib -lmysqlpp

uninstall-hook:
	-rmdir $(DESTDIR)$(pkglibdir)

