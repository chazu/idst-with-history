goals (and non-goals)
goals

REPL with read → expand → eval → print

message-passing only (no classes/metaclasses)

objects = dictionaries of selectors → C-implemented methods or closures

“image” = a graph of objects you can snapshot/restore

blocks and Booleans are real objects

a few core collections (List and Array) and Numbers

non-goals (v1)

full Smalltalk precedence/keyword message grammar

full GC (use libid’s memory discipline; add a simple mark/sweep later)

concurrency (add cooperative fibers later)

surface syntax (S72-lite)
Use a listy message packet to keep parsing trivial:

```
arduino(receiver selector arg …)
; examples:
(3 + 4)
(turtle forward: 100)
(if true [ (Transcript print: "yes") ] [ (Transcript print: "no") ])
(self do: [ (ball step) (Scheduler yield) ])
```

Atoms: foo, bar:, +, <=

Numbers: 123, -7, 3.14

Strings: "hello world"

Lists: ( … ) are message packets, not data; data lists are made with (List …) or literals '(a b c)

Blocks: [ … ] capture a chunk; they evaluate when sent value, value: x, etc.

Quotes: 'symbol (interned Symbol object)

Why not ST-72’s Englishy tokens? Because this list form keeps the reader tiny and makes the evaluator dead simple: “evaluate the first element to get the receiver, the second to a selector, the rest to arguments, then send.”

object model (on libid)

Object: has a method dictionary (selector → method) plus a parent slot for prototype-style delegation.

Selector: interned symbol.

Method: either

a C native (fast path), or

a Block (interpreted).

Send: send(receiver, selector, argc, argv[]) (macro/thin wrapper around libid’s message dispatch).

Lookup: current object → parent chain until found.

minimum types

Object (root; slots: parent, dict)

True, False, Nil

Number (boxed double for v1; add SmallInteger later)

String

Symbol (interned)

List (linked cells) and Array (fixed size)

Block (captures AST node + env)

Process (stubbed; add later)

Transcript (singleton I/O stream)

Turtle (fun demo)

evaluator

```
bashValue eval(AST node, Env* env):
  case Atom:       return lookupVar(env, atom) or interned Symbol
  case NumberLit:  return boxNumber(n)
  case StringLit:  return makeString(s)
  case Quote:      return quotedSymbol
  case Block:      return makeBlock(node, env)
  case List:
      Value rcv  = eval(node[0], env)
      Value sel  = eval(node[1], env)   ; must be Symbol
      Value args[] = map eval over node[2..]
      return send(rcv, sel, argc, args)
```

Blocks execute only upon value/value: send; capture env by reference.

Tail position: if last form of a block, you can tail-call by returning a thunk or by a trampoline (optional v1).

core protocol (tiny but useful)
Object

parent, parent: (prototype chain)

respondsTo:, perform:with: …

Booleans

ifTrue:, ifFalse:, ifTrue:ifFalse: (take Blocks)

and:, or:

Number

+ - * / = < > <= >=

print

List

isEmpty, first, rest, cons:, length, each:

Array

size, at:, at:put:, each:

Block

value, value:, value:value:

whileTrue:, whileFalse:

Transcript

print:, cr, show:

Turtle (demo)

forward:, turn:, penUp, penDown, goto:, clear

reader (tokenizer + parser)

Tokenize: parentheses, brackets, strings, numbers, symbols, quotes.

Parse ( … ) into a Message AST node (vector of AST children).

Parse [ … ] into a Block AST node; store its inner list of AST nodes.

Parse 'foo into a Quote node -> intern foo to a Symbol at eval time.

This is < ~300 lines of C.

libid wiring (sketch)

NOTE: function names/macros vary by libid version; adjust to your local header. The pattern below is the important part.

```c
c// selectors are interned once
static id SEL_PLUS, SEL_IFTRUE_IFFALSE, SEL_VALUE;

static void intern_selectors(void) {
  SEL_PLUS = id_intern("+");
  SEL_IFTRUE_IFFALSE = id_intern("ifTrue:ifFalse:");
  SEL_VALUE = id_intern("value");
}

static id send(id recv, id sel, int argc, id* argv) {
  // libid typically offers a generic send / dispatch; keep a single varargs gateway if available
  return id_sendN(recv, sel, argc, argv); // or a macro wrapper
}
```

method dictionary & prototypes

```c
ctypedef id (*cmethod_t)(id self, int argc, id* argv);

typedef struct Method {
  id selector;
  cmethod_t fun;     // native C impl
  id block;          // or a Block object if interpreted
} Method;

typedef struct Object {
  id parent;     // prototype delegate
  Map* dict;     // selector -> Method
  // optional slot map for instance vars
} Object;

static id obj_send(Object* self, id sel, int argc, id* argv) {
  for (Object* o=self; o; o=(Object*)o->parent) {
    Method* m = map_get(o->dict, sel);
    if (m) {
      if (m->fun) return m->fun((id)self, argc, argv);
      else        return eval_block(m->block, (id)self, argc, argv);
    }
  }
  return message_not_understood((id)self, sel, argc, argv);
}
```

Bind it to libid’s dispatch hook (or have send call obj_send for your own heap objects).

bootstrapping plan (milestones)
M0 — skeleton

build: cc -O2 -o s72 main.c reader.c eval.c libid.a

intern core selectors

atoms: Symbol intern table

boxed Number, String

REPL loop printing printString/Number literal

M1 — objects & send

Object with dict + parent

send + selector intern pool

install native methods for Number + - * / =

M2 — Booleans & control

singletons true, false, nil

ifTrue:ifFalse: taking two Blocks

Blocks capturing env; value, value:

M3 — Lists + Transcript

cons cell List + primitives

Transcript singleton with print: and cr

M4 — loader & “image”

read a startup file (init.s72) defining sugar and library in the language itself

simple snapshot: serialize object graph (IDs, type tags, dicts, arrays, strings) to a file; reload on start

M5 — demo actor

Turtle backed by a tiny raster (or just ASCII canvas initially)

example: draw a spiral via sends

tiny “standard library” (in language)
init.s72:

```
yaml; Booleans (already natives), add sugar
(def true  True)
(def false False)

(def not: [ (self ifTrue: [ false ] ifFalse: [ true ]) ])

; loops
(def times: [ :n :block
  (let i 0)
  ([ (< i n) ] whileTrue: [
     (block value: i)
     (set i (+ i 1))
  ])
])

; list map (primitive each: exists)
(def map: [ :lst :blk
  (if (lst isEmpty)
      [ '() ]
      [ (cons (blk value: (lst first))
              (map: (lst rest) blk)) ])
])
```

(Where def, let, set can be macros desugared by a tiny expander or built-ins for v1.)

REPL

```
shells72> (Transcript print: "hello") (Transcript cr)
hello
s72> (3 + 4)
7
s72> (true ifTrue: [ (Transcript print: "yay") ] ifFalse: [ (Transcript print: "nay") ])
yay
s72> (times: 5 [ :i (Transcript print: i) (Transcript cr) ])
0
1
2
3
4
```

example C: number “+” native

```c
cstatic id num_add(id self, int argc, id* argv) {
  if (argc != 1 || !isNumber(argv[0])) return type_error("+ expects 1 number");
  double a = asDouble(self), b = asDouble(argv[0]);
  return makeNumber(a + b);
}

static void install_number_prims(void) {
  id klass = NumberProto; // your Number prototype object
  install_native(klass, id_intern("+"), num_add);
  install_native(klass, id_intern("-"), num_sub);
  install_native(klass, id_intern("*"), num_mul);
  install_native(klass, id_intern("/"), num_div);
  install_native(klass, id_intern("="), num_eq);
}
```

minimal env/closures for Blocks

A Block stores: pointer to AST list, pointer to Env (a persistent map: symbol → value, with parent link).

On value/value:, create a child Env with the block’s parameter names bound to arguments, then eval each AST form in sequence.

Env sketch:

```c
ctypedef struct Env { Map* table; struct Env* parent; } Env;

id env_get(Env* e, id sym) {
  for (; e; e=e->parent) { id v; if (map_try(e->table, sym, &v)) return v; }
  return Nil;
}
```

optional niceties (v2)

Macros: one pass “expander” over AST before eval (just a map of selector → expander function/block).

Numbers: SmallInteger tagging + LargeInteger fallback.

Fibers: Process with a cooperative scheduler; yield, fork:.

FFI: expose C functions as CFunction objects; (FFI call: 'puts with: "hi").

Error objects: doesNotUnderstand:, assert:.

test plan (keep it tiny, fast)

Golden REPL scripts under tests/:

numbers.s72, booleans.s72, blocks.s72, lists.s72, turtle.s72

Runner executes each, captures stdout, diffs against .golden.

CI target: build + run tests in <2s.

project layout

```
bash/s72
  /src
    main.c          ; REPL, boot, init file loader
    reader.c/h      ; tokenizer + parser → AST
    ast.h           ; node kinds
    eval.c/h        ; eval + send
    object.c/h      ; Object/Method/Selector/Env
    number.c
    boolean.c
    list.c
    string.c
    block.c
    builtins.c      ; install_*() tables
    snapshot.c      ; serialize/deserialize
  /lib
    libid/          ; vendored or submodule
  /lang
    init.s72        ; stdlib in language
  /tests
    numbers.s72.golden
    ...
```

“day 1” bring-up checklist

wire up Symbol intern table & selector interns

implement Number, String boxes

minimal Object with dict + parent; send + lookup

reader for atoms/numbers/strings/lists/blocks

eval for each AST kind

natives for + - * / = and print:

REPL loop with Transcript output

load lang/init.s72 at start

tiny boot file you can start with
lang/init.s72

```
yaml(Transcript print: "s72 ready") (Transcript cr)

(def true True)
(def false False)

(def not: [ :b (b ifTrue: [ false ] ifFalse: [ true ]) ])

(def whileTrue: [ :cond :body
  ([ (cond value) ] whileTrue: [ (body value) ])
])
```
