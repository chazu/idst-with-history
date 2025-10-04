IDST   = /Users/chazu/dev/c/idst-with-history
CFLAGS = -g -Wall -Wreturn-type -Wno-error -fno-common  -g -O3 -fomit-frame-pointer -funroll-loops -I$(BIN)/include -DNDEBUG -DSYSARCH=\"arm64\" -DSYSOS=\"darwin\" -I/opt/homebrew/include $(EFLAGS) -fno-strict-aliasing
PFLAGS = -g -Wall -Wreturn-type -Wno-error -fno-common  -g -O3 -funroll-loops -I$(BIN)/include -DNDEBUG -DSYSARCH=\"arm64\" -DSYSOS=\"darwin\" -I/opt/homebrew/include $(EFLAGS) -fno-strict-aliasing
GFLAGS = -g -Wall -Wreturn-type -Wno-error -fno-common  -g           -I$(BIN)/include -DNDEBUG -DSYSARCH=\"arm64\" -DSYSOS=\"darwin\" -I/opt/homebrew/include $(EFLAGS) -fno-strict-aliasing
PREFIX = /usr/local/lib/idc/arm-apple-darwin23.6.0/
GCDIR  = system
SYSTEM_GC_LDFLAGS = -L/opt/homebrew/lib -lgc

LIBID_K_O = 
CCFLAGS_K =  -I$(BIN)/include -DNDEBUG -DSYSARCH=\"arm64\" -DSYSOS=\"darwin\" -I$(IDST)/system/include

all : $(BIN)./include $(BIN)libid.o $(BIN)libid_p.o $(BIN)libid_g.o $(BIN)$(LIBID_K_O)

$(BIN)./include :
	-$(SHELL) -ec 'mkdir $(BIN)./include; \
		mkdir $(BIN)./include/id;  cp -pr *.h $(BIN)./include/id; \
		if [ "$(GCDIR)" = "system" ]; then \
			mkdir -p $(BIN)./include/gc; \
			cp -pr /opt/homebrew/include/gc.h $(BIN)./include/gc/; \
			cp -pr /opt/homebrew/Cellar/bdw-gc/*/include/gc $(BIN)./include/gc/; \
		else \
			cp -pr ../$(GCDIR)/include $(BIN)./include/gc; \
		fi; \
		find $(BIN)./include/gc -name .svn -exec rm -rf {} \;'

$(BIN)libid.o : libid_enhanced.c $(BIN)./include
	$(CC) $(CFLAGS) -DPREFIX='"/usr/local/lib/idc/arm-apple-darwin23.6.0/"' $< -c -o $@

$(BIN)libid_p.o : libid_enhanced.c $(BIN)./include
	$(CC) $(PFLAGS) -pg -DPREFIX='"/usr/local/lib/idc/arm-apple-darwin23.6.0/"' $< -c -o $@

$(BIN)libid_g.o : libid_enhanced.c $(BIN)./include
	$(CC) $(GFLAGS) -g -DPREFIX='"/usr/local/lib/idc/arm-apple-darwin23.6.0/"' $< -c -o $@

$(BIN)libid_k.o : libid_enhanced.c $(BIN)./include
	$(CC) $(CCFLAGS_K) -DPREFIX='"/usr/local/lib/idc/arm-apple-darwin23.6.0/"' $< -c -o $@


install : $(PREFIX)libid.o $(PREFIX)libid_p.o $(PREFIX)libid_g.o

$(PREFIX)libid.o : $(BIN)libid.o
	-mkdir -p $(PREFIX)
	-cp -p $< $@

$(PREFIX)libid_p.o : $(BIN)libid_p.o
	-mkdir -p $(PREFIX)
	-cp -p $< $@

$(PREFIX)libid_g.o : $(BIN)libid_g.o
	-mkdir -p $(PREFIX)
	-cp -p $< $@

$(PREFIX)libid_k.o : $(BIN)libid_k.o
	-mkdir -p $(PREFIX)
	-cp -p $< $@


tidy : .FORCE
	rm -f *~

clean : tidy
	rm -f $(BIN)libid.o $(BIN)libid_p.o $(BIN)libid_g.o $(BIN)libid_k.o

distclean spotless : clean
	rm -f Makefile

.FORCE :
