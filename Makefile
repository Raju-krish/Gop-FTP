
all: gop_ftp.c ls_ftp.c gop_ftp.h
	@echo "Compiling sources for gop_ftp...."
	@gcc gop_ftp.c ls_ftp.c gop_ftp.h -o gop_ftp

clean:
	@echo "Removing executables..."
	@rm gop_ftp
