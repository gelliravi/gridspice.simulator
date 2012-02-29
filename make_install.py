#!/usr/bin/env python

# Developed by Yizheng Liao @ Jan 18th, 2012

# import modules
import os, sys, string, commands

if len(sys.argv) < 2:
	sys.exit('argument is not enough')
else:
	flag = sys.argv.pop()
	if flag == "configure":
		command="./configure 2> configure.error.log"
	elif flag == "make":
		command = "make 2> Makefile.error.log"
	elif flag == "install":
		command = "make install DESTDIR=/mnt/yizheng/ 2> install.error.log"


commandOutput = ""
try:
	print command + "\n"
	commandOutput += commands.getoutput(command) + "\n"
	outputInfo = string.split(commandOutput)

	print "Done!" + "\n"
except:
	sys.stderr.write()
