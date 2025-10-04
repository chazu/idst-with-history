ST80_FILES	= _object$(OBJEXT_O) \
		  Array$(OBJEXT_O) \
		  ArrayedCollection$(OBJEXT_O) \
		  Association$(OBJEXT_O) \
		  AVLTree$(OBJEXT_O) \
		  BlockClosure$(OBJEXT_O) \
		  ByteArray$(OBJEXT_O) \
		  Character$(OBJEXT_O) \
		  Collection$(OBJEXT_O) \
		  Dictionary$(OBJEXT_O) \
		  Fraction$(OBJEXT_O) \
		  Float$(OBJEXT_O) \
		  GetOpt$(OBJEXT_O) \
		  IdentityDictionary$(OBJEXT_O) \
		  IdentitySet$(OBJEXT_O) \
		  Integer$(OBJEXT_O) \
		  LargeInteger$(OBJEXT_O) \
		  LargeNegativeInteger$(OBJEXT_O) \
		  LargePositiveInteger$(OBJEXT_O) \
		  LookupKey$(OBJEXT_O) \
		  Magnitude$(OBJEXT_O) \
		  Number$(OBJEXT_O) \
		  Object$(OBJEXT_O) \
		  OrderedCollection$(OBJEXT_O) \
		  PositionableStream$(OBJEXT_O) \
		  ReadStream$(OBJEXT_O) \
		  SequenceableCollection$(OBJEXT_O) \
		  Set$(OBJEXT_O) \
		  SmallInteger$(OBJEXT_O) \
		  Smalltalk$(OBJEXT_O) \
		  Stream$(OBJEXT_O) \
		  String$(OBJEXT_O) \
		  Symbol$(OBJEXT_O) \
		  WordArray$(OBJEXT_O) \
		  WriteStream$(OBJEXT_O) \
		  perform$(OBJEXT_O) \
		  tokenization$(OBJEXT_O)

IDST_FILES	= ArgumentVariableNode$(OBJEXT_O) \
		  ArrayLiteralNode$(OBJEXT_O) \
		  ArrayNode$(OBJEXT_O) \
		  AssignmentNode$(OBJEXT_O) \
		  BlockNode$(OBJEXT_O) \
		  ByteArrayNode$(OBJEXT_O) \
		  CodeGenerator$(OBJEXT_O) \
		  CCodeGenerator$(OBJEXT_O) \
		  CManagedCodeGenerator$(OBJEXT_O) \
		  CharacterNode$(OBJEXT_O) \
		  Compiler$(OBJEXT_O) \
		  CompilerOptions$(OBJEXT_O) \
		  DefinitionNode$(OBJEXT_O) \
		  EncodedVariableNode$(OBJEXT_O) \
		  Encoder$(OBJEXT_O) \
		  ExecNode$(OBJEXT_O) \
		  ExternNode$(OBJEXT_O) \
		  FloatNode$(OBJEXT_O) \
		  GlobalVariableNode$(OBJEXT_O) \
		  ImportNode$(OBJEXT_O) \
		  IncludeNode$(OBJEXT_O) \
		  IntegerNode$(OBJEXT_O) \
		  LiteralNode$(OBJEXT_O) \
		  LocalVariableNode$(OBJEXT_O) \
		  MethodNode$(OBJEXT_O) \
		  ParseNode$(OBJEXT_O) \
		  Parser$(OBJEXT_O) \
		  PrimitiveNode$(OBJEXT_O) \
		  PrototypeNode$(OBJEXT_O) \
		  ReturnNode$(OBJEXT_O) \
		  Scanner$(OBJEXT_O) \
		  ScannerContext$(OBJEXT_O) \
		  ScannerPosition$(OBJEXT_O) \
		  Scope$(OBJEXT_O) \
		  SendNode$(OBJEXT_O) \
		  SequenceNode$(OBJEXT_O) \
		  SlotVariableNode$(OBJEXT_O) \
		  StringIO$(OBJEXT_O) \
		  StringNode$(OBJEXT_O) \
		  SymbolNode$(OBJEXT_O) \
		  TemporaryVariableNode$(OBJEXT_O) \
		  VarargNode$(OBJEXT_O) \
		  VariableNode$(OBJEXT_O) \
		  WordArrayNode$(OBJEXT_O) \

FILES		= include/gc \
		  st80$(OBJEXT_SO) \
		  idst$(OBJEXT_SO) \
		  idc1$(OBJEXT) \
		  idc \
		  gc.a

TARGET		= arm-apple-darwin23.6.0
PREFIX		= /usr/local/lib/idc/arm-apple-darwin23.6.0/
CC		= cc
CFLAGS		= -g -Wall -Wreturn-type -Wno-error -fno-common  -DNDEBUG -DSYSARCH=\"arm64\" -DSYSOS=\"darwin\" $(EFLAGS)
MFLAGS		= 
OFLAGS		= -O
O3FLAGS		= -O3 -fomit-frame-pointer -funroll-loops -fno-strict-aliasing
CCFLAGS		= 
CCFLAGS_O	= -c
CCFLAGS_SO	= 
LDFLAGS		= 
LDFLAGS_O	= 
LDFLAGS_SO	= -bundle -flat_namespace -undefined suppress
LDLIBS		= 
LDLIBS_O	= 
LDLIBS_SO	= 
OBJEXT		= 
OBJEXT_O	= .o
OBJEXT_SO	= .so
GCDIR		= system
SYSTEM_GC_CFLAGS = -I/opt/homebrew/include
SYSTEM_GC_LDFLAGS = -L/opt/homebrew/lib -lgc
SYSOS		= darwin

all : $(FILES)

libid.o : src/libid.c include/gc gc.a
	$(CC) -Iinclude $(SYSTEM_GC_CFLAGS) -DPREFIX='"$(PREFIX)"' $(CFLAGS) $(O3FLAGS) $(CCFLAGS_O) $(LDFLAGS_O) $< $(LDLIBS_O) $(SYSTEM_GC_LDFLAGS) -o $@

libid_g.o : src/libid.c include/gc gc.a
	$(CC) -Iinclude $(SYSTEM_GC_CFLAGS) -DPREFIX='"$(PREFIX)"' $(CFLAGS) $(GFLAGS) $(CCFLAGS_O) $(LDFLAGS_O) $< $(LDLIBS_O) $(SYSTEM_GC_LDFLAGS) -o $@

idc : ../idc/idc gc.a
	cp -p $< .

.SUFFIXES :

st80$(OBJEXT_SO) : src/st80$(OBJEXT_SO).c $(ST80_FILES) include/gc gc.a
	$(CC) -Iinclude $(SYSTEM_GC_CFLAGS) $(CFLAGS) $(OFLAGS) $(CCFLAGS_SO) $(LDFLAGS_SO) $< $(ST80_FILES) $(LDLIBS_SO) $(SYSTEM_GC_LDFLAGS) -o $@

idst$(OBJEXT_SO) : src/idst$(OBJEXT_SO).c $(IDST_FILES) include/gc gc.a
	$(CC) -Iinclude $(SYSTEM_GC_CFLAGS) $(CFLAGS) $(OFLAGS) $(CCFLAGS_SO) $(LDFLAGS_SO) $< $(IDST_FILES) $(LDLIBS_SO) $(SYSTEM_GC_LDFLAGS) -o $@

%$(OBJEXT_O) : src/%$(OBJEXT_O).c include/gc gc.a
	$(CC) -Iinclude $(SYSTEM_GC_CFLAGS) $(CFLAGS) $(OFLAGS) $(CCFLAGS_O) $(LDFLAGS_O) $< $(LDLIBS_O) $(SYSTEM_GC_LDFLAGS) -o $@

%$(OBJEXT) : src/%.c gc.a libid.o libid_g.o include/gc
	$(CC) -Iinclude $(SYSTEM_GC_CFLAGS) $(CFLAGS) $(OFLAGS) $(CCFLAGS) $(LDFLAGS) $< $(LDLIBS) $(SYSTEM_GC_LDFLAGS) -o $@ libid.o gc.a -lm

include/gc :
ifeq ($(GCDIR),system)
	rm -rf include/gc
	mkdir -p include/gc
	cp -pr /opt/homebrew/include/gc.h include/gc/
	cp -pr /opt/homebrew/Cellar/bdw-gc/*/include/gc include/gc/
else
	rm -rf include/gc
	cp -pr ../$(GCDIR)/include include/gc
endif

gc.a :
ifeq ($(GCDIR),system)
	-$(SHELL) -ec 'ln -s /opt/homebrew/lib/libgc.a ./gc.a'
else
	-$(SHELL) -ec 'ln -s ../$(GCDIR)/.libs/libgc.a ./gc.a'
endif

../$(GCDIR)/.libs/libgc.a :
ifeq ($(SYSOS),win32)
	-$(SHELL) -ec 'cd ../$(GCDIR)/; $(MAKE) -f Makefile.win32 ABI_FLAG="$(MFLAGS) -DLARGE_CONFIG"'
else
	-$(SHELL) -ec 'cd ../$(GCDIR)/; echo $$PATH; test -f Makefile || CFLAGS="$(MFLAGS)" ./configure --enable-large-config --disable-shared; $(MAKE)'
endif

install : all .FORCE
	-rm -rf $(PREFIX)
	-mkdir -p $(PREFIX)
	-cp -pr include gc.a libid.o libid_g.o idc idc1 *$(OBJEXT_SO) $(PREFIX)/.
	-cp -pr ../st80/*.st $(PREFIX)/.
	-cp -pr ../idc/*.st $(PREFIX)/.
	-cp -pr idc /usr/local/bin/.
	-find $(PREFIX)/. -name .svn -exec rm -rf {} \;

tidy : .FORCE
	-rm -f *.d *~ .gdb* *.stackdump

clean : tidy
	-$(SHELL) -ec 'cd ../$(GCDIR)/; $(MAKE) clean'
	-rm -rf $(FILES) idc1$(OBJEXT) gc.a *$(OBJEXT_O) *$(OBJEXT_SO)

distclean spotless : clean
	-$(SHELL) -ec 'cd ../$(GCDIR)/; $(MAKE) distclean'
	-$(SHELL) -ec 'cd ../$(GCDIR)/libatomic_ops; $(MAKE) distclean'
	-rm -f Makefile

.FORCE :
