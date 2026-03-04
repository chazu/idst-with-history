/* asm-x86_64.h -- dynamic assembler for x86_64
 *
 * Based on asm-i386.h by Ian Piumarta.
 * Extended for x86_64 with REX prefix support and 64-bit registers.
 */

#ifndef __ccg_asm_x86_64_h
#define __ccg_asm_x86_64_h

#include <sys/types.h>
#include <stdint.h>

typedef unsigned char insn;

#include "asm-common.h"

#define _b00		0
#define _b01		1
#define _b10		2
#define _b11		3

#define _b000		0
#define _b001		1
#define _b010		2
#define _b011		3
#define _b100		4
#define _b101		5
#define _b110		6
#define _b111		7


/*** REGISTERS ***/	/* [size,,number] */

/* 8-bit low registers */
#define _AL		0x10
#define _CL		0x11
#define _DL		0x12
#define _BL		0x13
/* REX-accessible low byte registers */
#define _SPL		0x14
#define _BPL		0x15
#define _SIL		0x16
#define _DIL		0x17

/* 16-bit registers */
#define _AX		0x20
#define _CX		0x21
#define _DX		0x22
#define _BX		0x23
#define _SP		0x24
#define _BP		0x25
#define _SI		0x26
#define _DI		0x27

/* 32-bit registers */
#define _EAX		0x40
#define _ECX		0x41
#define _EDX		0x42
#define _EBX		0x43
#define _ESP		0x44
#define _EBP		0x45
#define _ESI		0x46
#define _EDI		0x47

/* 64-bit registers (encoded with size nibble 0x8) */
#define _RAX		0x80
#define _RCX		0x81
#define _RDX		0x82
#define _RBX		0x83
#define _RSP		0x84
#define _RBP		0x85
#define _RSI		0x86
#define _RDI		0x87
#define _R8		0x88
#define _R9		0x89
#define _R10		0x8a
#define _R11		0x8b
#define _R12		0x8c
#define _R13		0x8d
#define _R14		0x8e
#define _R15		0x8f

#define _rS(R)		((R)>>4)
#define _rN(R)		((R)&0x07)
/* For 64-bit regs: low 3 bits for ModR/M encoding */
#define _rN8(R)		((R)&0x07)
/* Predicate: is this an extended register (r8-r15)? */
#define _rXP(R)		(((R)&0x08)!=0)

#define _r0P(R)		((R)==0)
#define _r1P(R)		(_rS(R)==1)
#define _r2P(R)		(_rS(R)==2)
#define _r4P(R)		(_rS(R)==4)
#define _r8P(R)		(_rS(R)==8)

#define _r1(R)		(_r1P(R) ? _rN(R) : ASMFAIL( "8-bit register required"))
#define _r2(R)		(_r2P(R) ? _rN(R) : ASMFAIL("16-bit register required"))
#define _r4(R)		(_r4P(R) ? _rN(R) : ASMFAIL("32-bit register required"))
#define _r8(R)		(_r8P(R) ? _rN8(R) : ASMFAIL("64-bit register required"))

#define _rAL(R)		(((R)==_AL)  ? _rN(R) : ASMFAIL( "AL register required"))
#define _rAX(R)		(((R)==_AX)  ? _rN(R) : ASMFAIL( "AX register required"))
#define _rEAX(R)	(((R)==_EAX) ? _rN(R) : ASMFAIL("EAX register required"))
#define _rRAX(R)	(((R)==_RAX) ? _rN8(R): ASMFAIL("RAX register required"))


/*** IMMEDIATES ***/

#define _s0P(I)		((I)==0)
#define _s1P(I)		(((int)(I)) >= -128 && ((int)(I)) < 128)
#define _s4P(I)		(((long)(I)) >= -2147483648L && ((long)(I)) <= 2147483647L)

#define _u1P(I)		(((I) & ~0xffL) == 0)
#define _u2P(I)		(((I) & ~0xffffL) == 0)

#define _s1(I)		(_s1P(I) ? ((uint8_t)(I)) : ASMFAIL( "8-bit signed immediate required"))


/*** ASSEMBLER ***/

#ifndef _GEN

  static int _B(int x) {
    uint8_t *pc= (uint8_t *)asm_pc;
    if (asm_pass) *pc= (uint8_t)x;
    asm_pc= (insn *)++pc;
    return x;
  }

  static int _W(int x) {
    uint16_t *pc= (uint16_t *)asm_pc;
    if (asm_pass) *pc= (uint16_t)x;
    asm_pc= (insn *)++pc;
    return x;
  }

  static int _L(int x) {
    uint32_t *pc= (uint32_t *)asm_pc;
    if (asm_pass) *pc= (uint32_t)x;
    asm_pc= (insn *)++pc;
    return x;
  }

  static long _Q(long x) {
    uint64_t *pc= (uint64_t *)asm_pc;
    if (asm_pass) *pc= (uint64_t)x;
    asm_pc= (insn *)++pc;
    return x;
  }

# define _OFF4(D)	((uint32_t)(long)(D) - (uint32_t)(long)asm_pc)
# define _CKD1(D)	((asm_pass==1) ? _OFF4(D) : (uint8_t)_OFF4(D))

  static int8_t _D1(long d) {
    int8_t *pc= (int8_t *)asm_pc;
    int8_t off= 0;
    _B(0);
    off= (int8_t)_CKD1(d);
    if (asm_pass) *pc= off;
    return off;
  }

  static int32_t _D4(long d) {
    int32_t *pc= (int32_t *)asm_pc;
    int32_t off= 0;
    _L(0);
    off= _OFF4(d);
    if (asm_pass) *pc= off;
    return off;
  }

#endif /* !_GEN */


/*** REX PREFIX ***/

/* REX byte: 0100WRXB
 *   W = 64-bit operand size
 *   R = ModR/M reg field extension
 *   X = SIB index extension
 *   B = ModR/M r/m or SIB base extension
 */
#define _REX(W,R,X,B)		_B(0x40|((W)<<3)|((R)<<2)|((X)<<1)|(B))

/* REX.W only (64-bit operand) */
#define _REXW()			_B(0x48)
/* REX.W + REX.R (reg field is extended register) */
#define _REXWR(R)		_B(0x48|(_rXP(R)?0x04:0))
/* REX.W + REX.B (r/m or base is extended register) */
#define _REXWB(B)		_B(0x48|(_rXP(B)?0x01:0))
/* REX.W + REX.R + REX.B */
#define _REXWRB(R,B)		_B(0x48|(_rXP(R)?0x04:0)|(_rXP(B)?0x01:0))
/* REX.W + REX.R + REX.X + REX.B (for SIB with all extensions) */
#define _REXWRXB(R,X,B)	_B(0x48|(_rXP(R)?0x04:0)|(_rXP(X)?0x02:0)|(_rXP(B)?0x01:0))
/* REX prefix needed for SPL/BPL/SIL/DIL access (REX with no flags set) */
#define _REX0()			_B(0x40)


/*** SAFETY CHECKS ***/

#define _r(R)		((R)&0x07)
#define _M(M)		(M)
#define _m(M)		(M)
#define _s(S)		(S)
#define _i(I)		(I)
#define _rb(B)		(B)

#define _Mrm(Md,R,M)	_B((_M(Md)<<6)|(_r(R)<<3)|_m(M))
#define _SIB(Sc,I, B)	_B((_s(Sc)<<6)|(_i(I)<<3)|_rb(B))

#define _SCL(S)		((((S)==1) ? _b00 : \
			 (((S)==2) ? _b01 : \
			 (((S)==4) ? _b10 : \
			 (((S)==8) ? _b11 : ASMFAIL("illegal scale: " #S))))))


/*** MEMORY SUBFORMATS (64-bit aware) ***/

/* 64-bit: [disp32] uses RIP-relative addressing (mod=00, r/m=101)
 * For absolute addresses we must use SIB with base=none
 * But for this JIT, we mostly use register-relative, so keep the i386 patterns */

#define _r_D(	R, D	  )		(_Mrm(_b00,_rN8(R),_b101 )		             ,_L(D))
#define _r_0B(	R,   B    )		(_Mrm(_b00,_rN8(R),_rN8(B))			           )
#define _r_0BIS(R,   B,I,S)		(_Mrm(_b00,_rN8(R),_b100 ),_SIB(_SCL(S),_rN8(I),_rN8(B))      )
#define _r_0DIS(R, D,  I,S)		(_Mrm(_b00,_rN8(R),_b100 ),_SIB(_SCL(S),_rN8(I),    5 ),_L(D))
#define _r_1B(	R, D,B    )		(_Mrm(_b01,_rN8(R),_rN8(B))		             ,_B(D))
#define _r_1BIS(R, D,B,I,S)		(_Mrm(_b01,_rN8(R),_b100 ),_SIB(_SCL(S),_rN8(I),_rN8(B)),_B(D))
#define _r_4B(	R, D,B    )		(_Mrm(_b10,_rN8(R),_rN8(B))		             ,_L(D))
#define _r_4BIS(R, D,B,I,S)		(_Mrm(_b10,_rN8(R),_b100 ),_SIB(_SCL(S),_rN8(I),_rN8(B)),_L(D))

#define _r_DB(  R, D,B    )		((_s0P(D) ? _r_0B  (R,  B    ) : (_s1P(D) ? _r_1B(  R,D,B    ) : _r_4B(  R,D,B    ))))
#define _r_DBIS(R, D,B,I,S)		((_s0P(D) ? _r_0BIS(R,  B,I,S) : (_s1P(D) ? _r_1BIS(R,D,B,I,S) : _r_4BIS(R,D,B,I,S))))

/* Special handling: RSP (r/m=100) needs SIB, RBP (r/m=101) with disp=0 needs disp8=0 */
#define _r_X(   R, D,B,I,S)		(_r0P(I)  ? (_r0P(B)   ? _r_D   (R,D            )   : \
					            (_rN8(B)==4 ? _r_DBIS(R,D,_rN8(B),4,1)   : \
					             (_rN8(B)==5 && _s0P(D) ? _r_1B(R,0,B)   : \
							         _r_DB  (R,D,   B       )))) : \
					 ((_r0P(B) ?             _r_0DIS(R,D,        I,S)   : \
					  ((_rN8(I)!=4)        ? _r_DBIS(R,D,   B,   I,S)   : \
							         ASMFAIL("illegal index register: rsp")))))


/*** INSTRUCTION FORMAT HELPERS ***/

/*	_format		Opcd		ModR/M dN(rB,rI,Sc)	imm... */

#define  _O(        OP                         )  (		  _B(  OP       )                                 )
#define  _Or(       OP,R                       )  (		  _B( (OP)|_r(R))                                 )
#define  _OO(       OP                         )  ( _B((OP)>>8), _B( (OP)      )                                 )
#define  _OOr(      OP,R                       )  ( _B((OP)>>8), _B( (OP)|_r(R))                                 )
#define  _Os(       OP,B                       )  (    _s1P(B) ? _B(((OP)|_b10)) : _B(OP)                        )
#define     _sL(                             L  )  (		                       _s1P(L) ? _B(L):_L(L)      )
#define  _O_D4(     OP                     ,D  )  (        _O      (  OP  )                         ,_D4(D)      )
#define  _OO_D4(    OP                     ,D  )  (       _OO      (  OP  )                         ,_D4(D)      )
#define  _Os_sL(    OP                     ,L  )  (        _Os     (  OP,L)                         ,_sL(L)      )
#define  _Or_L(     OP,R                   ,L  )  (        _Or     (  OP,R)                          ,_L(L)      )
#define  _Or_Q(     OP,R                   ,Q  )  (        _Or     (  OP,R)                          ,_Q(Q)      )
#define  _O_Mrm(    OP  ,MO,R,M                )  (        _O      (  OP  ),_Mrm(MO,R,M            )             )
#define  _OO_Mrm(   OP  ,MO,R,M                )  (       _OO      (  OP  ),_Mrm(MO,R,M            )             )
#define  _O_Mrm_B(  OP  ,MO,R,M            ,B  )  (        _O      (  OP  ),_Mrm(MO,R,M            ) ,_B(B)      )
#define  _O_Mrm_L(  OP  ,MO,R,M            ,L  )  (        _O      (  OP  ),_Mrm(MO,R,M            ) ,_L(L)      )
#define  _Os_Mrm_sL(OP  ,MO,R,M            ,L  )  (        _Os     (  OP,L),_Mrm(MO,R,M            ),_sL(L)      )
#define  _O_r_X(    OP     ,R  ,MD,MB,MI,MS    )  (        _O      (  OP  ),_r_X(   R  ,MD,MB,MI,MS)             )
#define  _OO_r_X(   OP     ,R  ,MD,MB,MI,MS    )  (       _OO      (  OP  ),_r_X(   R  ,MD,MB,MI,MS)             )
#define  _O_r_X_B(  OP     ,R  ,MD,MB,MI,MS,B  )  (        _O      (  OP  ),_r_X(   R  ,MD,MB,MI,MS) ,_B(B)      )
#define  _O_r_X_L(  OP     ,R  ,MD,MB,MI,MS,L  )  (        _O      (  OP  ),_r_X(   R  ,MD,MB,MI,MS) ,_L(L)      )
#define  _Os_r_X_sL(OP     ,R  ,MD,MB,MI,MS,L  )  (        _Os     (  OP,L),_r_X(   R  ,MD,MB,MI,MS),_sL(L)      )


/*** 64-BIT (QUAD) INSTRUCTION MACROS ***/

/* The pattern: emit REX.W (with R/B extensions as needed), then the same
 * encoding as the 32-bit instruction but using _rN8() for register fields. */


/* --- ADD --- */

#define ADDQir(IM, RD)			(_REXWB(RD),		_Os_Mrm_sL	(0x81		,_b11,_b000  ,_rN8(RD)			,IM	))
#define ADDQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x01		,_b11,_rN8(RS),_rN8(RD)				))
#define ADDQmr(MD, MB, MI, MS, RD)	(_REXWRXB(RD,MI,MB),	_O_r_X		(0x03		     ,_rN8(RD)		,MD,MB,MI,MS		))
#define ADDQrm(RS, MD, MB, MI, MS)	(_REXWRXB(RS,MI,MB),	_O_r_X		(0x01		     ,_rN8(RS)		,MD,MB,MI,MS		))

/* --- SUB --- */

#define SUBQir(IM, RD)			(_REXWB(RD),		_Os_Mrm_sL	(0x81		,_b11,_b101  ,_rN8(RD)			,IM	))
#define SUBQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x29		,_b11,_rN8(RS),_rN8(RD)				))

/* --- MOV --- */

/* movq reg, reg */
#define MOVQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x89		,_b11,_rN8(RS),_rN8(RD)				))
/* movq (mem), reg */
#define MOVQmr(MD, MB, MI, MS, RD)	(_REXWRXB(RD,MI,MB),	_O_r_X		(0x8b		     ,_rN8(RD)		,MD,MB,MI,MS		))
/* movq reg, (mem) */
#define MOVQrm(RS, MD, MB, MI, MS)	(_REXWRXB(RS,MI,MB),	_O_r_X		(0x89		     ,_rN8(RS)		,MD,MB,MI,MS		))
/* movabs imm64, reg (REX.W + B8+r, followed by 8-byte immediate) */
#define MOVQir(IM, RD)			(_REXWB(RD),		_Or_Q		(0xb8,_rN8(RD)					,(long)(IM)	))
/* movq imm32(sign-ext), reg */
#define MOVQi32r(IM, RD)		(_REXWB(RD),		_O_Mrm_L	(0xc7		,_b11,_b000  ,_rN8(RD)			,IM	))

/* --- LEA --- */

#define LEAQmr(MD, MB, MI, MS, RD)	(_REXWRXB(RD,MI,MB),	_O_r_X		(0x8d		     ,_rN8(RD)		,MD,MB,MI,MS		))

/* --- CMP --- */

#define CMPQir(IM, RD)			(_REXWB(RD),		_Os_Mrm_sL	(0x81		,_b11,_b111  ,_rN8(RD)			,IM	))
#define CMPQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x39		,_b11,_rN8(RS),_rN8(RD)				))
#define CMPQim(IM, MD, MB, MI, MS)	(_REXWRXB(0,MI,MB),	_O_r_X_L	(0x81		     ,_b111		,MD,MB,MI,MS	,IM	))

/* --- TEST --- */

#define TESTQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x85		,_b11,_rN8(RS),_rN8(RD)				))

/* --- AND --- */

#define ANDQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x21		,_b11,_rN8(RS),_rN8(RD)				))
#define ANDQir(IM, RD)			(_REXWB(RD),		_Os_Mrm_sL	(0x81		,_b11,_b100  ,_rN8(RD)			,IM	))

/* --- OR --- */

#define ORQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x09		,_b11,_rN8(RS),_rN8(RD)				))

/* --- XOR --- */

#define XORQrr(RS, RD)			(_REXWRB(RS,RD),	_O_Mrm		(0x31		,_b11,_rN8(RS),_rN8(RD)				))

/* --- IMUL --- */

#define IMULQrr(RS,RD)			(_REXWRB(RD,RS),	_OO_Mrm		(0x0faf		,_b11,_rN8(RD),_rN8(RS)				))

/* --- IDIV --- */

#define IDIVQr(RS)			(_REXWB(RS),		_O_Mrm		(0xf7		,_b11,_b111  ,_rN8(RS)				))

/* --- NEG --- */

#define NEGQr(RD)			(_REXWB(RD),		_O_Mrm		(0xf7		,_b11,_b011  ,_rN8(RD)				))

/* --- NOT --- */

#define NOTQr(RD)			(_REXWB(RD),		_O_Mrm		(0xf7		,_b11,_b010  ,_rN8(RD)				))

/* --- Shifts --- */

/* shlq %cl, RD */
#define SHLQrr(RS,RD)		(((RS)==_CL) ?	(_REXWB(RD),	_O_Mrm		(0xd3	,_b11,_b100,_rN8(RD)				)) : \
							ASMFAIL("CL required for shift"))
/* salq %cl, RD  (same as shl) */
#define SALQrr(RS,RD)			SHLQrr(RS,RD)

/* shrq %cl, RD */
#define SHRQrr(RS,RD)		(((RS)==_CL) ?	(_REXWB(RD),	_O_Mrm		(0xd3	,_b11,_b101,_rN8(RD)				)) : \
							ASMFAIL("CL required for shift"))

/* sarq %cl, RD */
#define SARQrr(RS,RD)		(((RS)==_CL) ?	(_REXWB(RD),	_O_Mrm		(0xd3	,_b11,_b111,_rN8(RD)				)) : \
							ASMFAIL("CL required for shift"))

/* --- PUSH/POP (no REX.W needed; default 64-bit in long mode) --- */

#define PUSHQr(R)		(_rXP(R) ? (_B(0x41), _O(0x50|_rN8(R))) : _O(0x50|_rN8(R)))
#define POPQr(R)		(_rXP(R) ? (_B(0x41), _O(0x58|_rN8(R))) : _O(0x58|_rN8(R)))

/* --- CQTO (sign-extend rax into rdx:rax) --- */

#define CQTO()				(_REXW(),		_O		(0x99								))

/* --- RET --- */

#define RET()				_O		(0xc3								)

/* --- CALL --- */

/* call *disp(base) or call rel32 */
#define CALLm(MD,MB,MI,MS)		((_r0P(MB) && _r0P(MI)) ? _O_D4 (0xe8			,(long)(MD)		) : \
					(_r0P(MI) ? (_rXP(MB) ? (_B(0x41),_O_Mrm(0xff,_b11,_b010,_rN8(MB))) : \
								 _O_Mrm(0xff,_b11,_b010,_rN8(MB))	) : \
					 (_rXP(MB) ? (_B(0x41), _O_r_X(0xff,_b010,(int)(MD),MB,MI,MS)) : \
								 _O_r_X(0xff,_b010,(int)(MD),MB,MI,MS)	)))

/* --- JMP --- */

#define JMPm(D,B,I,S)			((_r0P(B) && _r0P(I)) ? _O_D4	(0xe9			,(long)(D)		) : \
					(_r0P(I) ? (_rXP(B) ? (_B(0x41),_O_Mrm(0xff,_b11,_b100,_rN8(B))) : \
								 _O_Mrm(0xff,_b11,_b100,_rN8(B))	) : \
					 ASMFAIL("jmp: unsupported addressing mode")))

/* --- Jcc (conditional jumps, 32-bit displacement) --- */

#define JCCim(CC,D,B,I,S)		((_r0P(B) && _r0P(I)) ? _OO_D4	(0x0f80|(CC)		,(long)(D)		) : \
								ASMFAIL("conditional jump: unsupported addressing mode"))

#define JEm(D,B,I,S)			JCCim(0x4,D,B,I,S)
#define JNEm(D,B,I,S)			JCCim(0x5,D,B,I,S)
#define JLm(D,B,I,S)			JCCim(0xc,D,B,I,S)
#define JGEm(D,B,I,S)			JCCim(0xd,D,B,I,S)
#define JLEm(D,B,I,S)			JCCim(0xe,D,B,I,S)
#define JGm(D,B,I,S)			JCCim(0xf,D,B,I,S)

/* --- SETcc (byte result from condition codes) --- */
/* These work the same as i386 — no REX needed for al/cl/dl/bl */

#define SETCCir(CC,RD)			_OO_Mrm		(0x0f90|(CC)	,_b11,_b000,_rN(RD)				)

#define SETEr(RD)			SETCCir(0x4,RD)
#define SETNEr(RD)			SETCCir(0x5,RD)
#define SETLr(RD)			SETCCir(0xc,RD)
#define SETGEr(RD)			SETCCir(0xd,RD)
#define SETLEr(RD)			SETCCir(0xe,RD)
#define SETGr(RD)			SETCCir(0xf,RD)

/* --- Byte/word operations (used for asgni1, asgni2, indiri1, indiri2) --- */

/* movb RS, (mem) — no REX.W (8-bit operation), but need REX.B for extended regs */
#define MOVBrm(RS, MD, MB, MI, MS)	_O_r_X		(0x88		     ,_rN(RS)		,MD,MB,MI,MS		)
/* movw RS, (mem) */
#define MOVWrm(RS, MD, MB, MI, MS)	(_B(0x66),		_O_r_X		(0x89		     ,_rN(RS)		,MD,MB,MI,MS		))

/* movsbq (mem), RD  — sign-extend byte to 64 bits */
#define MOVSBQmr(MD,MB,MI,MS,RD)	(_REXWRXB(RD,MI,MB),	_OO_r_X		(0x0fbe		     ,_rN8(RD)		,MD,MB,MI,MS		))
/* movswq (mem), RD  — sign-extend word to 64 bits */
#define MOVSWQmr(MD,MB,MI,MS,RD)	(_REXWRXB(RD,MI,MB),	_OO_r_X		(0x0fbf		     ,_rN8(RD)		,MD,MB,MI,MS		))

/* i386-compat aliases used by the code generator */
#define MOVSBLmr(MD,MB,MI,MS,RD)	MOVSBQmr(MD,MB,MI,MS,RD)
#define MOVSWLmr(MD,MB,MI,MS,RD)	MOVSWQmr(MD,MB,MI,MS,RD)

/* --- 32-bit instructions (for compatibility / i386-style macro names used in codegen) --- */

#define MOVLir(IM, R)			_Or_L		(0xb8,_rN8(R)						,IM	)
#define MOVLrr(RS, RD)			_O_Mrm		(0x89		,_b11,_rN8(RS),_rN8(RD)				)
#define MOVLmr(MD, MB, MI, MS, RD)	_O_r_X		(0x8b		     ,_rN8(RD)		,MD,MB,MI,MS		)
#define MOVLrm(RS, MD, MB, MI, MS)	_O_r_X		(0x89		     ,_rN8(RS)		,MD,MB,MI,MS		)
#define ADDLir(IM, RD)			_Os_Mrm_sL	(0x81		,_b11,_b000  ,_rN8(RD)			,IM	)
#define ADDLrr(RS, RD)			_O_Mrm		(0x01		,_b11,_rN8(RS),_rN8(RD)				)
#define SUBLir(IM, RD)			_Os_Mrm_sL	(0x81		,_b11,_b101  ,_rN8(RD)			,IM	)
#define CMPLir(IM, RD)			_O_Mrm_L	(0x81		,_b11,_b111  ,_rN8(RD)			,IM	)
#define CMPLrr(RS, RD)			_O_Mrm		(0x39		,_b11,_rN8(RS),_rN8(RD)				)
#define TESTLrr(RS, RD)			_O_Mrm		(0x85		,_b11,_rN8(RS),_rN8(RD)				)
#define XORLrr(RS, RD)			_O_Mrm		(0x31		,_b11,_rN8(RS),_rN8(RD)				)
#define NEGLr(RD)			_O_Mrm		(0xf7		,_b11,_b011  ,_rN8(RD)				)
#define NOTLr(RD)			_O_Mrm		(0xf7		,_b11,_b010  ,_rN8(RD)				)
#define LEALmr(MD, MB, MI, MS, RD)	_O_r_X		(0x8d		     ,_rN8(RD)		,MD,MB,MI,MS		)
#define IDIVLrr(RS,RD)		( _rEAX(RD) +	_O_Mrm	(0xf7		,_b11,_b111  ,_rN8(RS)				) )
#define IMULLrr(RS,RD)			_OO_Mrm		(0x0faf		,_b11,_rN8(RD),_rN8(RS)				)
#define ANDLrr(RS, RD)			_O_Mrm		(0x21		,_b11,_rN8(RS),_rN8(RD)				)
#define ORLrr(RS, RD)			_O_Mrm		(0x09		,_b11,_rN8(RS),_rN8(RD)				)
#define SHLLrr(RS,RD)		(((RS)==_CL) ?	_O_Mrm		(0xd3	,_b11,_b100,_rN8(RD)				) : \
							ASMFAIL("CL required for shift"))
#define SALLrr(RS,RD)			SHLLrr(RS,RD)
#define SARLrr(RS,RD)		(((RS)==_CL) ?	_O_Mrm		(0xd3	,_b11,_b111,_rN8(RD)				) : \
							ASMFAIL("CL required for shift"))
#define SHRLrr(RS,RD)		(((RS)==_CL) ?	_O_Mrm		(0xd3	,_b11,_b101,_rN8(RD)				) : \
							ASMFAIL("CL required for shift"))
#define PUSHLr(R)			_Or		(0x50,_rN8(R)							)
#define POPLr(RD)			_Or		(0x58,_rN8(RD)							)
#define CLTD()				_O		(0x99								)

/* Generic push/pop that dispatch on register size */
#define PUSHr(R)			(_r8P(R) ? PUSHQr(R) : _r4P(R) ? PUSHLr(R) : ASMFAIL("illegal register size for push"))
#define POPr(R)				(_r8P(R) ? POPQr(R) : _r4P(R) ? POPLr(R) : ASMFAIL("illegal register size for pop"))


#endif /* __ccg_asm_x86_64_h */
