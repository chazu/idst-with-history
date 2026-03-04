" CodeGenerator-x86_64.st -- translate abstract Instructions to x86_64 machine instructions

  Based on CodeGenerator-i386.st by Ian Piumarta.
  Extended for x86_64 with System V AMD64 ABI calling convention.

  Copyright (c) 2006, 2007 Ian Piumarta
  All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the 'Software'),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, provided that the above copyright notice(s) and this
  permission notice appear in all copies of the Software and that both the
  above copyright notice(s) and this permission notice appear in supporting
  documentation.

  THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
"

{ import: Objects }
{ import: Expression }
{ import: Resource }
{ import: Instruction }
{ import: CompilerOptions }
{ import: CodeGenerator }

{ include ""asm-x86_64.h"" }

Intel64CodeGenerator : CodeGenerator ( tempsSize rax rcx rdx rbx rsp rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 cx cl )

Intel64CodeGenerator new
[
    self := super new.
    tempsSize := 0.
    "caller-saved registers (clobbered by calls)"
    ccrs
	add: (rax := Register withClass: I4 name: '%rax' encoding: 0x80);
	add: (rcx := Register withClass: I4 name: '%rcx' encoding: 0x81);
	add: (rdx := Register withClass: I4 name: '%rdx' encoding: 0x82);
	add: (rsi := Register withClass: I4 name: '%rsi' encoding: 0x86);
	add: (rdi := Register withClass: I4 name: '%rdi' encoding: 0x87);
	add: (r8  := Register withClass: I4 name: '%r8'  encoding: 0x88);
	add: (r9  := Register withClass: I4 name: '%r9'  encoding: 0x89);
	add: (r10 := Register withClass: I4 name: '%r10' encoding: 0x8a);
	add: (r11 := Register withClass: I4 name: '%r11' encoding: 0x8b).
    "callee-saved registers"
    csrs
	add: (rbx := Register withClass: I4 name: '%rbx' encoding: 0x83);
	add: (r12 := Register withClass: I4 name: '%r12' encoding: 0x8c);
	add: (r13 := Register withClass: I4 name: '%r13' encoding: 0x8d);
	add: (r14 := Register withClass: I4 name: '%r14' encoding: 0x8e);
	add: (r15 := Register withClass: I4 name: '%r15' encoding: 0x8f).
    	      rsp := Register withClass: P4 name: '%rsp' encoding: 0x84.
    	      rbp := Register withClass: P4 name: '%rbp' encoding: 0x85.
    	      cx  := Register withClass: I2 name: '%cx'  encoding: 0x21.
    	      cl  := Register withClass: I1 name: '%cl'  encoding: 0x11.
]

Intel64CodeGenerator clobberRegisters: insn	[ insn clobberIntel64: self ]

Instruction clobberIntel64: gen	[]

ADDRFP4 clobberIntel64: gen [ gen requireFrame ]
ASGNI1	clobberIntel64: gen [ gen clobberRCX:   self ]
ASGNI2	clobberIntel64: gen [ gen clobberRCX:   self ]
Call	clobberIntel64: gen [ gen clobberCallRegs: self; requireFrame ]
DIVI4	clobberIntel64: gen [ gen clobberRADX: 	self; requireFrame ]
MODI4	clobberIntel64: gen [ gen clobberRADX: 	self; requireFrame ]
EQI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
LEI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
LTI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
GEI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
GTI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
NEI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
NOTI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
SHLI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
SHRI4	clobberIntel64: gen [ gen clobberRCX:  	self ]
SHLU4	clobberIntel64: gen [ gen clobberRCX:  	self ]
SHRU4	clobberIntel64: gen [ gen clobberRCX:  	self ]

"Calls clobber all caller-saved registers"
Intel64CodeGenerator clobberCallRegs: insn
[
    self clobberRegister: rax; clobberRegister: rcx; clobberRegister: rdx;
	 clobberRegister: rsi; clobberRegister: rdi;
	 clobberRegister: r8;  clobberRegister: r9;
	 clobberRegister: r10; clobberRegister: r11
]
Intel64CodeGenerator clobberRADX: insn	[ self clobberRegister: rax; clobberRegister: rdx ]
Intel64CodeGenerator clobberRCX:  insn	[ self clobberRegister: rcx ]

Intel64CodeGenerator allocateParam: insn
[
    ^(params add: (Temporary withBase: rsp offset: params size * 8)) allocate
]

Intel64CodeGenerator allocateSpill: insn
[
    spills do: [:temp | temp live ifFalse: [^temp allocate]].
    ^(spills add: (Temporary withBase: rsp offset: 0)) allocate
]

Intel64CodeGenerator allocateTemp: insn
[
    temps do: [:temp | temp live ifFalse: [^temp allocate]].
    ^(temps add: (Temporary withBase: rsp offset: 0)) allocate
]

Intel64CodeGenerator finaliseFrame
[
    | align |
    frameSize := 8.							"return address"
    tempsSize := 0.
    needsFrame ifTrue: [frameSize := frameSize + 8].			"saved frame pointer"
    csrs do: [:reg | reg used ifTrue: [frameSize := frameSize + 8]].	"callee-saved register"
    temps do: [:temp |
	temp offset: tempsSize + parametersSize.
	tempsSize := tempsSize + 8].					"temporary variable"
    spills do: [:temp |
	temp offset: tempsSize + parametersSize.
	tempsSize := tempsSize + 8].					"temporary variable"
    tempsSize := tempsSize + parametersSize.				"outgoing parameters"
    frameSize := frameSize + tempsSize.
    needsFrame
	ifTrue:
	   [align := (frameSize + 15 bitAnd: -16) - frameSize.
	    tempsSize := tempsSize + align.
	    frameSize := frameSize + align].
    needsFrame
	ifTrue:  [params do: [:param | param base: rbp; offset: param offset + 16]]
	ifFalse: [params do: [:param | param offset: param offset + frameSize]].
]

Intel64CodeGenerator emitPrologue
[
    needsFrame ifTrue: [self pushqR: rbp;  movqR: rsp R: rbp].
    csrs do: [:reg | reg used ifTrue: [self pushqR: reg]].
    tempsSize > 0 ifTrue: [self subqI: tempsSize R: rsp].
]

Intel64CodeGenerator emitEpilogue
[
    tempsSize > 0 ifTrue: [self addqI: tempsSize R: rsp].
    csrs reverseDo: [:reg | reg used ifTrue: [self popqR: reg]].
    needsFrame ifTrue: [self popqR: rbp].
    self ret.
]

Intel64CodeGenerator noteCall: call
[
    | stackArgs |
    "System V AMD64: first 6 integer args in rdi,rsi,rdx,rcx,r8,r9; rest on stack"
    stackArgs := call arity - 6 max: 0.
    parametersSize := parametersSize max: stackArgs * 8.
]

Intel64CodeGenerator calli4: call
[
    | func nargs |
    nargs := call arity.
    "push stack args (7th arg onwards) right to left"
    nargs - 1 downTo: 6 do: [:index | self emit: call stackArgumentAt: index].
    "load register args"
    nargs - 1 min: 5 downTo: 0 do: [:index | self emit: call registerArgumentAt: index].
    (func := call function) reload: self.
    self callMb: func output.
    self movqR: rax R: call output.
]

Intel64CodeGenerator emit: call registerArgumentAt: index
[
    | insn dst |
    (insn := call argumentAt: index) reload: self.
    dst := self registerForArgument: index.
    self movqR: insn output R: dst.
]

Intel64CodeGenerator emit: call stackArgumentAt: index
[
    | insn |
    (insn := call argumentAt: index) reload: self.
    self movqR: insn output Md: (index - 6) * 8 b: rsp.
]

Intel64CodeGenerator registerForArgument: index
[
    index == 0 ifTrue: [^rdi].
    index == 1 ifTrue: [^rsi].
    index == 2 ifTrue: [^rdx].
    index == 3 ifTrue: [^rcx].
    index == 4 ifTrue: [^r8].
    index == 5 ifTrue: [^r9].
    self error: 'registerForArgument: index out of range'
]

"----------------------------------------------------------------"

StaticIntel64CodeGenerator : Intel64CodeGenerator ()

StaticIntel64CodeGenerator addrgp4  :d :v	[ StdOut nextPutAll: '	movabsq	'; nextPut: $$; print: v; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator addrjp4  :d :l	[ StdOut nextPutAll: '	movabsq	$L'; print: l ordinal; nextPut: $,; print: d; cr ]

StaticIntel64CodeGenerator addqI: i R: d	[ StdOut nextPutAll: '	addq	'; nextPut: $$; print: i; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator addqI_: i R: d	[ StdOut nextPutAll: '	addq	'; nextPut: $$; print: (SmallInteger value_: i); nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator addqR: s R: d	[ StdOut nextPutAll: '	addq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator andqR: s R: d	[ StdOut nextPutAll: '	andq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator callMb: b		[ StdOut nextPutAll: '	call	*('; print: b; nextPutAll: ')\n' ]
StaticIntel64CodeGenerator cqto			[ StdOut nextPutAll: '	cqto\n' ]
StaticIntel64CodeGenerator cmpqI: i R: d	[ StdOut nextPutAll: '	cmpq	'; nextPut: $$; print: i; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator cmpqI_: i Md: d b: b	[ StdOut nextPutAll: '	cmpq	'; nextPut: $$; print: (SmallInteger value_: i); nextPut: $,; print: d; nextPut: $(; print: b; nextPut: $); cr]
StaticIntel64CodeGenerator cmpqR: s R: d	[ StdOut nextPutAll: '	cmpq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator define: l		[ StdOut nextPutAll: 'L'; print: l ordinal; nextPut: $:; cr ]
StaticIntel64CodeGenerator idivqR: s		[ StdOut nextPutAll: '	idivq	'; print: s; cr ]
StaticIntel64CodeGenerator imulqR: s R: d	[ StdOut nextPutAll: '	imulq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator jeL: l		[ StdOut nextPutAll: '	je	L'; print: l ordinal; cr ]
StaticIntel64CodeGenerator jgeL: l		[ StdOut nextPutAll: '	jge	L'; print: l ordinal; cr ]
StaticIntel64CodeGenerator jmpL: l		[ StdOut nextPutAll: '	jmp	L'; print: l ordinal; cr ]
StaticIntel64CodeGenerator jmpMb: b		[ StdOut nextPutAll: '	jmp	*'; print: b; cr ]
StaticIntel64CodeGenerator jneL: l		[ StdOut nextPutAll: '	jne	L'; print: l ordinal; cr ]
StaticIntel64CodeGenerator leaqMd: d b: b R: r	[ StdOut nextPutAll: '	leaq	'; print: d; nextPut: $(; print: b; nextPutAll: '),'; print: r; cr]
StaticIntel64CodeGenerator leaqMd_: d b: b R: r	[ StdOut nextPutAll: '	leaq	'; print: (SmallInteger value_: d); nextPut: $(; print: b; nextPutAll: '),'; print: r; cr]
StaticIntel64CodeGenerator movbR: r Mb: b	[ StdOut nextPutAll: '	movb	'; print: r; nextPutAll: ',('; print: b; nextPut: $); cr ]
StaticIntel64CodeGenerator movqI_: i R: d	[ StdOut nextPutAll: '	movabsq	'; nextPut: $$; print: (SmallInteger value_: i); nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator movqMb: b R: r	[ StdOut nextPutAll: '	movq	('; print: b; nextPutAll: '),'; print: r; cr]
StaticIntel64CodeGenerator movqMd: d b: b R: r	[ StdOut nextPutAll: '	movq	'; print: d; nextPut: $(; print: b; nextPutAll: '),'; print: r; cr]
StaticIntel64CodeGenerator movqMd_: d R: r	[ StdOut nextPutAll: '	movq	'; print: (SmallInteger value_: d); nextPut: $,; print: r; cr ]
StaticIntel64CodeGenerator movqR: r Mb: b	[ StdOut nextPutAll: '	movq	'; print: r; nextPutAll: ',('; print: b; nextPut: $); cr ]
StaticIntel64CodeGenerator movqR: s Md: d b: b	[ StdOut nextPutAll: '	movq	'; print: s; nextPut: $,; print: d; nextPut: $(; print: b; nextPut: $); cr]
StaticIntel64CodeGenerator movqR: s R: d	[ s == d ifFalse: [StdOut nextPutAll: '	movq	'; print: s; nextPut: $,; print: d; cr] ]
StaticIntel64CodeGenerator movsbqMb: s R: d	[ StdOut nextPutAll: '	movsbq	('; print: s; nextPutAll: '),'; print: d; cr ]
StaticIntel64CodeGenerator movswqMb: s R: d	[ StdOut nextPutAll: '	movswq	('; print: s; nextPutAll: '),'; print: d; cr ]
StaticIntel64CodeGenerator movwR: r Mb: b	[ StdOut nextPutAll: '	movw	'; print: r; nextPutAll: ',('; print: b; nextPut: $); cr ]
StaticIntel64CodeGenerator negqR: d		[ StdOut nextPutAll: '	negq	'; print: d; cr ]
StaticIntel64CodeGenerator notqR: d		[ StdOut nextPutAll: '	notq	'; print: d; cr ]
StaticIntel64CodeGenerator orqR: s R: d		[ StdOut nextPutAll: '	orq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator popqR: d		[ StdOut nextPutAll: '	popq	'; print: d; cr ]
StaticIntel64CodeGenerator pushqR: s		[ StdOut nextPutAll: '	pushq	'; print: s; cr ]
StaticIntel64CodeGenerator ret			[ StdOut nextPutAll: '	ret\n' ]
StaticIntel64CodeGenerator salqR: s R: d	[ StdOut nextPutAll: '	salq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator sarqR: s R: d	[ StdOut nextPutAll: '	sarq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator seteR: d		[ StdOut nextPutAll: '	sete	'; print: d; cr ]
StaticIntel64CodeGenerator setgR: d		[ StdOut nextPutAll: '	setg	'; print: d; cr ]
StaticIntel64CodeGenerator setgeR: d		[ StdOut nextPutAll: '	setge	'; print: d; cr ]
StaticIntel64CodeGenerator setlR: d		[ StdOut nextPutAll: '	setl	'; print: d; cr ]
StaticIntel64CodeGenerator setleR: d		[ StdOut nextPutAll: '	setle	'; print: d; cr ]
StaticIntel64CodeGenerator setneR: d		[ StdOut nextPutAll: '	setne	'; print: d; cr ]
StaticIntel64CodeGenerator shlqR: s R: d	[ StdOut nextPutAll: '	shlq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator shrqR: s R: d	[ StdOut nextPutAll: '	shrq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator subqI: i R: d	[ StdOut nextPutAll: '	subq	'; nextPut: $$; print: i; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator subqI_: i R: d	[ StdOut nextPutAll: '	subq	'; nextPut: $$; print: (SmallInteger value_: i); nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator subqR: s R: d	[ StdOut nextPutAll: '	subq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator testqR: s R: d	[ StdOut nextPutAll: '	testq	'; print: s; nextPut: $,; print: d; cr ]
StaticIntel64CodeGenerator xorqR: s R: d	[ StdOut nextPutAll: '	xorq	'; print: s; nextPut: $,; print: d; cr ]

Intel64CodeGenerator spillq:  reg to:   tmp	[ self movqR:  reg        Md: tmp offset b: rsp ]
Intel64CodeGenerator reloadq: reg from: tmp	[ self movqMd: tmp offset  b: rsp        R: reg ]

Intel64CodeGenerator spilli4:  reg to:   tmp	[ self movqR:  reg        Md: tmp offset b: rsp ]
Intel64CodeGenerator reloadi4: reg from: tmp	[ self movqMd: tmp offset  b: rsp        R: reg ]

"----------------------------------------------------------------"

DynamicIntel64CodeGenerator : Intel64CodeGenerator ()

DynamicIntel64CodeGenerator asmPass	{ _return (oop)(asm_pass << 1 | 1); }
DynamicIntel64CodeGenerator _asmPC	{ _return (oop)(asm_pc); }

SmallInteger _int	{ _return (oop)((long)self >> 1); }

DynamicIntel64CodeGenerator define: l
[
    self asmPass == 0
	ifTrue:  [l address_: self _asmPC]
	ifFalse: [l phaseCheck_: self _asmPC]
]

DynamicIntel64CodeGenerator _
{
    (void)_W; (void)_D1; (void)_D4; (void)_Q;
#   undef _P
#   undef _I
#   undef _R
#   undef _A
#   define _P(v)	((long)v__##v)
#   define _I(v)	((long)v_##v >> 1)
#   define _R(v)	((long)(((struct t_Register *)(v_##v))->v__encoding))
#   define _A(v)	((long)(((struct t_Label    *)(v_##v))->v__address))
}

DynamicIntel64CodeGenerator addqI: i R: d	  { ADDQir	(_I(i)          ,	_R(d)	 ); }
DynamicIntel64CodeGenerator addqI_: _i R: d	  { ADDQir	(_P(i)		,	_R(d)	 ); }
DynamicIntel64CodeGenerator addqR: s R: d	  { ADDQrr	(      _R(s)	, 	_R(d)    ); }
DynamicIntel64CodeGenerator andqR: s R: d	  { ANDQrr	(      _R(s)	, 	_R(d)    ); }
DynamicIntel64CodeGenerator cqto		  { CQTO	(				 ); }
DynamicIntel64CodeGenerator callMb: b		  { CALLm	(    0,_R(b),0,0		 ); }
DynamicIntel64CodeGenerator cmpqI: i R: d	  { CMPQir	(_I(i)          ,	_R(d)	 ); }
DynamicIntel64CodeGenerator cmpqI_: _i Md: d b: b { CMPQim	(_P(i)		, _I(d),_R(b),0,0); }
DynamicIntel64CodeGenerator cmpqR: s R: d	  { CMPQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator idivqR: s		  { IDIVQr	(      _R(s)			 ); }
DynamicIntel64CodeGenerator imulqR: s R: d	  { IMULQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator jeL: l		  { JEm		(_A(l),    0,0,0		 ); }
DynamicIntel64CodeGenerator jgeL: l		  { JGEm	(_A(l),	   0,0,0		 ); }
DynamicIntel64CodeGenerator jmpMb: b		  { JMPm	(    0,_R(b),0,0		 ); }
DynamicIntel64CodeGenerator jmpL: l		  { JMPm	(_A(l),	   0,0,0		 ); }
DynamicIntel64CodeGenerator jneL: l		  { JNEm	(_A(l),	   0,0,0		 ); }
DynamicIntel64CodeGenerator leaqMd: d b: b R: r	  { LEAQmr	(_I(d),_R(b),0,0,	_R(r)	 ); }
DynamicIntel64CodeGenerator leaqMd_: _d b: b R: r { LEAQmr	(_P(d),_R(b),0,0,	_R(r)	 ); }
DynamicIntel64CodeGenerator movbR: r Mb: b	  { MOVBrm	(      _R(r)	,     0,_R(b),0,0); }
DynamicIntel64CodeGenerator movqI_: _i R: d	  { MOVQir	(_P(i)		,	_R(d)	 ); }
DynamicIntel64CodeGenerator movqMb: b R: r	  { MOVQmr	(    0,_R(b),0,0,	_R(r)	 ); }
DynamicIntel64CodeGenerator movqMd: d b: b R: r	  { MOVQmr	(_I(d),_R(b),0,0,	_R(r)	 ); }
DynamicIntel64CodeGenerator movqMd_: _d R: r	  { MOVQmr	(_P(d),    0,0,0,	_R(r)	 ); }
DynamicIntel64CodeGenerator movqR: r Mb: b	  { MOVQrm	(      _R(r)	,     0,_R(b),0,0); }
DynamicIntel64CodeGenerator movqR: s Md: d b: b	  { MOVQrm	(      _R(s)	, _I(d),_R(b),0,0); }
DynamicIntel64CodeGenerator movqR: s R: d	  { MOVQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator movsbqMb: s R: d	  { MOVSBQmr	(    0,_R(s),0,0, 	_R(d)	 ); }
DynamicIntel64CodeGenerator movswqMb: s R: d	  { MOVSWQmr	(    0,_R(s),0,0, 	_R(d)	 ); }
DynamicIntel64CodeGenerator movwR: r Mb: b	  { MOVWrm	(      _R(r)	,     0,_R(b),0,0); }
DynamicIntel64CodeGenerator negqR: d		  { NEGQr	(      			_R(d)	 ); }
DynamicIntel64CodeGenerator notqR: d		  { NOTQr	(      			_R(d)	 ); }
DynamicIntel64CodeGenerator orqR: s R: d	  { ORQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator popqR: d		  { POPQr	(      			_R(d)	 ); }
DynamicIntel64CodeGenerator pushqR: s		  { PUSHQr	(      _R(s)			 ); }
DynamicIntel64CodeGenerator ret			  { RET		(				 ); }
DynamicIntel64CodeGenerator salqR: s R: d	  { SALQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator sarqR: s R: d	  { SARQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator seteR: d		  { SETEr	(			_R(d)	 ); }
DynamicIntel64CodeGenerator setgR: d		  { SETGr	(			_R(d)	 ); }
DynamicIntel64CodeGenerator setgeR: d		  { SETGEr	(			_R(d)	 ); }
DynamicIntel64CodeGenerator setlR: d		  { SETLr	(			_R(d)	 ); }
DynamicIntel64CodeGenerator setleR: d		  { SETLEr	(			_R(d)	 ); }
DynamicIntel64CodeGenerator setneR: d		  { SETNEr	(			_R(d)	 ); }
DynamicIntel64CodeGenerator shlqR: s R: d	  { SHLQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator shrqR: s R: d	  { SHRQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator subqI: i R: d	  { SUBQir	(_I(i)		,	_R(d)	 ); }
DynamicIntel64CodeGenerator subqI_: _i R: d	  { SUBQir	(_P(i)		,	_R(d)	 ); }
DynamicIntel64CodeGenerator subqR: s R: d	  { SUBQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator testqR: s R: d	  { TESTQrr	(      _R(s)	,	_R(d)	 ); }
DynamicIntel64CodeGenerator xorqR: s R: d	  { XORQrr	(      _R(s)	,	_R(d)	 ); }

"----------------------------------------------------------------"

CodeGenerator versionString	[ ^self default versionString ]

CodeGenerator default		[ ^Intel64CodeGenerator ]

Intel64CodeGenerator static	[ ^StaticIntel64CodeGenerator ]
Intel64CodeGenerator dynamic	[ ^DynamicIntel64CodeGenerator ]

Intel64CodeGenerator versionString
[
    ^'x86_64 1.0 generic'
]

"----------------------------------------------------------------"

CodeGenerator		   isStatic	[ ^false ]
StaticIntel64CodeGenerator isStatic	[ ^true ]

CodeGenerator		    isDynamic	[ ^false ]
DynamicIntel64CodeGenerator isDynamic	[ ^true ]

StaticIntel64CodeGenerator defineVariable: name
[
    StdOut nextPutAll: '	.data\n'.
    StdOut nextPutAll: name; nextPutAll: ':	.quad	_'; nextPutAll: name; cr.
    StdOut nextPutAll: '	.text\n'.
]

DynamicIntel64CodeGenerator defineVariable: name
[
]

"----------------------------------------------------------------"

{ import: ReductionGrammar }

Intel64CodeGenerator cl		[ ^cl  ]
Intel64CodeGenerator cx		[ ^cx  ]
Intel64CodeGenerator rax	[ ^rax ]
Intel64CodeGenerator rcx	[ ^rcx ]
Intel64CodeGenerator rdx	[ ^rdx ]
Intel64CodeGenerator rbp	[ ^rbp ]

Intel64Grammar := [
    ReductionGrammar new
	"-------------------------------- baseline --------------------------------"

	at: #VOID add: #REG			"discard";
	at: #VOID add: #(enter		)	do: [:op :gen | gen emitPrologue ];
	at: #VOID add: #(retv		)	do: [:op :gen | gen emitEpilogue ];
	at: #VOID add: #(reti4 	 REG	)	do: [:op :gen | gen movqR:    op lhs output   R: gen rax; emitEpilogue ];
	at: #VOID add: #(drop  	 REG	)	do: [:op :gen | ];
	at: #VOID add: #(brz   	 REG	)	do: [:op :gen | gen testqR:   op lhs output   R: op lhs output;  jeL: op destination ];
	at: #VOID add: #(brnz  	 REG	)	do: [:op :gen | gen testqR:   op lhs output   R: op lhs output;  jneL: op destination ];
	at: #VOID add: #(bri 	 REG	)	do: [:op :gen | gen jmpMb:    op lhs output ];
	at: #VOID add: #(bra		)	do: [:op :gen | gen jmpL:     op destination ];
	at: #VOID add: #(label		)	do: [:op :gen | gen define: op ];
	at: #REG  add: #(cnsti4	 	)	do: [:op :gen | gen movqI_:   op arg	      R: op output ];
	at: #REG  add: #(cnstp4	 	)	do: [:op :gen | gen movqI_:   op arg	      R: op output ];
	at: #REG  add: #(parami4 	)	do: [:op :gen | ];
	at: #REG  add: #(addrfp4 	)	do: [:op :gen | gen movqR:    gen rbp	      R: op output ];
	at: #REG  add: #(addrgp4 	)	do: [:op :gen | gen movqI_:   op arg _address R: op output ];
	at: #REG  add: #(addrjp4 	)	do: [:op :gen | gen movqI_:   op arg _address R: op output ];
	at: #REG  add: #(addrlp4 	)	do: [:op :gen | gen leaqMd:   op arg offset   b: op arg base R: op output ];
	at: #REG  add: #(indiri1 REG	)	do: [:op :gen | gen movsbqMb: op lhs output   R: op output ];
	at: #REG  add: #(indiri2 REG	)	do: [:op :gen | gen movswqMb: op lhs output   R: op output ];
	at: #REG  add: #(indiri4 REG	)	do: [:op :gen | gen movqMb:   op lhs output   R: op output ];
	at: #REG  add: #(noti4 	 REG	)	do: [:op :gen | gen xorqR:    gen rcx	      R: gen rcx;
							    cmpqI:    0		      R: op output;
							    seteR:    gen cl;
							    movqR:    gen rcx	      R: op output ];
	at: #REG  add: #(comi4 	 REG	)	do: [:op :gen | gen notqR:    op output ];
	at: #REG  add: #(negi4 	 REG	)	do: [:op :gen | gen negqR:    op output ];
	at: #REG  add: #(calli4	 REG	)	do: [:op :gen | gen calli4: op ];
	at: #REG  add: #(asgni1  REG REG)	do: [:op :gen | gen movqR:    op lhs output   R: gen rcx;
							    movbR:    gen cl	     Mb: op rhs output ];
	at: #REG  add: #(asgni2  REG REG)	do: [:op :gen | gen movqR:    op lhs output   R: gen rcx;
							    movwR:    gen cx	     Mb: op rhs output ];
	at: #REG  add: #(asgni4  REG REG)	do: [:op :gen | gen movqR:    op lhs output  Mb: op rhs output ];
	at: #REG  add: #(andi4   REG REG)	do: [:op :gen | gen andqR:    op rhs output   R: op output ];
	at: #REG  add: #(ori4    REG REG)	do: [:op :gen | gen orqR:     op rhs output   R: op output ];
	at: #REG  add: #(xori4 	 REG REG)	do: [:op :gen | gen xorqR:    op rhs output   R: op output ];
	at: #REG  add: #(addi4 	 REG REG)	do: [:op :gen | gen addqR:    op rhs output   R: op output ];
	at: #REG  add: #(subi4 	 REG REG)	do: [:op :gen | gen subqR:    op rhs output   R: op output ];
	at: #REG  add: #(muli4 	 REG REG)	do: [:op :gen | gen imulqR:   op rhs output   R: op output ];
	at: #REG  add: #(divi4 	 REG REG)	do: [:op :gen | gen movqR:    op lhs output   R: gen rax;
							    cqto;
							    idivqR: op rhs output;
							    movqR:  gen rax	      R: op output ];
	at: #REG  add: #(modi4 	 REG REG)	do: [:op :gen | gen movqR:  op lhs output     R: gen rax;
							    cqto;
							    idivqR: op rhs output;
							    movqR:  gen rdx	  R: op output ];
	at: #REG  add: #(shli4 	 REG REG)	do: [:op :gen | gen movqR:  op rhs output R: gen rcx;
							    salqR:  gen cl	  R: op output ];
	at: #REG  add: #(shlu4 	 REG REG)	do: [:op :gen | gen movqR:  op rhs output R: gen rcx;
							    shlqR:  gen cl	  R: op output ];
	at: #REG  add: #(shri4 	 REG REG)	do: [:op :gen | gen movqR:  op rhs output R: gen rcx;
							    sarqR:  gen cl	  R: op output ];
	at: #REG  add: #(shru4 	 REG REG)	do: [:op :gen | gen movqR:  op rhs output R: gen rcx;
							    shrqR:  gen cl	  R: op output ];
	at: #REG  add: #(lti4 	 REG REG)	do: [:op :gen | gen xorqR:  gen rcx	  R: gen rcx;
							    cmpqR:  op rhs output R: op lhs output;
							    setlR:  gen cl;
							    movqR:  gen rcx	  R: op output ];
	at: #REG  add: #(lei4 	 REG REG)	do: [:op :gen | gen xorqR:  gen rcx	  R: gen rcx;
							    cmpqR:  op rhs output R: op lhs output;
							    setleR: gen cl;
							    movqR:  gen rcx	  R: op output ];
	at: #REG  add: #(eqi4 	 REG REG)	do: [:op :gen | gen xorqR:  gen rcx	  R: gen rcx;
							    cmpqR:  op rhs output R: op lhs output;
							    seteR:  gen cl;
							    movqR:  gen rcx	  R: op output ];
	at: #REG  add: #(nei4 	 REG REG)	do: [:op :gen | gen xorqR:  gen rcx	  R: gen rcx;
							    cmpqR:  op rhs output R: op lhs output;
							    setneR: gen cl;
							    movqR:  gen rcx	  R: op output ];
	at: #REG  add: #(gei4 	 REG REG)	do: [:op :gen | gen xorqR:  gen rcx	  R: gen rcx;
							    cmpqR:  op rhs output R: op lhs output;
							    setgeR: gen cl;
							    movqR:  gen rcx	  R: op output ];
	at: #REG  add: #(gti4 	 REG REG)	do: [:op :gen | gen xorqR:  gen rcx	  R: gen rcx;
							    cmpqR:  op rhs output R: op lhs output;
							    setgR:  gen cl;
							    movqR:  gen rcx	  R: op output ];

	"-------------------------------- optimisations --------------------------------"

	at: #VOID add: #(brz (lti4 (indiri4 (addrlp4))
			           (cnsti4)))		do: [:op :gen | gen cmpqI_:  op lhs rhs arg Md: op lhs lhs lhs arg offset b: op lhs lhs lhs arg base;
								    jgeL:    op destination ];

	at: #REG  add: #(indiri4 (addrlp4))		do: [:op :gen | gen movqMd:  op lhs arg offset b: op lhs arg base R: op output ];
	at: #REG  add: #(indiri4 (addrgp4))		do: [:op :gen | gen movqMd_: op lhs arg _address R: op output ];
	at: #REG  add: #(subi4 REG (cnsti4))		do: [:op :gen | gen subqI_:  op rhs arg R: op lhs output ];
	at: #REG  add: #(addi4 REG (cnsti4))		do: [:op :gen | gen addqI_:  op rhs arg R: op lhs output ];
	at: #REG  add: #(addi4 (cnsti4) REG)		do: [:op :gen | gen leaqMd_: op lhs arg b: op rhs output R: op output ];

	yourself
]

"----------------------------------------------------------------"

Intel64RegisterFilter    : RegisterAllocator ()
Intel64RegisterAllocator : RegisterAllocator ()

Intel64RegisterFilter	 REG  :insn	[ ^true ]
Intel64RegisterAllocator REG  :insn	[ ^codeGenerator allocateRegister: insn ]

Intel64CodeGenerator grammar		[ ^Intel64Grammar ]
Intel64CodeGenerator registerFilter	[ ^Intel64RegisterFilter   "on: self"]
Intel64CodeGenerator registerAllocator	[ ^Intel64RegisterAllocator on: self ]

"----------------------------------------------------------------"

StaticIntel64CodeGenerator generate: tree
[
    self finaliseFrame.
    tree printOn: StdOut indent: 0.  StdOut cr.
    tree generate: self.
    ^Array with: 0 with: 0.
]

"{ include ""dis-x86_64.c"" }"

DynamicIntel64CodeGenerator generate: tree
[
    | _entry _size |
    { asm_pass= 0; }.
    { asm_pc= 0; }.
    self finaliseFrame.
    tree generate: self.
    CompilerOptions verboseExec ifTrue: [{ printf("code size %d bytes\n", (int)(long)asm_pc); }].
    { v__size= (oop)asm_pc; }.
    { asm_pc= (insn *)malloc((long)asm_pc); }.
    { asm_pass= 2; }.
    { v__entry= (oop)asm_pc; }.
    self relocateLabels_: self _asmPC.
    tree generate: self.
    CompilerOptions verboseExec ifTrue: [{ printf("code start %p\n", (void *)v__entry); }].
    { iflush((void *)v__entry, (void *)asm_pc); }.
"    CompilerOptions verboseExec ifTrue: [{ disassemble((void *)v__entry, (void *)asm_pc); }]."
    ^Array with: _entry with: _size.
]
