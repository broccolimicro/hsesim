Usage: hsesim [options] file...
A simulation environment for HSE processes.

Supported file formats:
 *.hse           Load an HSE
 *.dot           Load any place-transition graph
 *.sim           Load a sequence of transitions to operate on

General Options:
 -h,--help      Display this information
    --version   Display version information
 -v,--verbose   Display verbose messages
 -d,--debug     Display internal debugging messages

Conversion Options:
 -g <file>      Convert this HSE to an hse-graph and save it to a file
 -eg <file>     Convert this HSE to an elaborated hse-graph and save it to a file
 -pn <file>     Convert this HSE to a petri-net and save it to a file
 -sg <file>     Convert this HSE to a state-graph and save it to a file

=========================== Simulation Environment ============================

In order to get to the simulation environment run the following command:

hsesim file.hse

It will bring you to a prompt that looks like this:

(hsesim)

From there you can execute any of the following commands:

<arg> specifies a required argument
(arg=value) specifies an optional argument with a default value

General:
 help, h             print this message
 seed <n>            set the random seed for the simulation
 source <file>       source and execute a list of commands from a file
 save <file>         save the sequence of fired transitions to a '.sim' file
 load <file>         load a sequence of transitions from a '.sim' file
 clear, c            clear any stored sequence and return to random stepping
 quit, q             exit the interactive simulation environment

Running Simulation:
 tokens, t           list the location and state information of every token
 enabled, e          return the list of enabled transitions
 fire <i>, f<i>      fire the i'th enabled transition
 step (N=1), s(N=1)  step through N transitions (random unless a sequence is loaded)
 reset (i), r(i)     reset the simulator to the initial marking and re-seed (does not clear)

Setting/Viewing State:
 set <i> <expr>      execute a transition as if it were local to the i'th token
 set <expr>          execute a transition as if it were remote to all tokens
 force <expr>        execute a transition as if it were local to all tokens

The following example will be using this hse:

R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1

To begin, view set of possible reset states and then pick one.

(hsesim)r
(0) {P0 P9 P15} {} ~R.f&~R.t&L.e&R.e&~L.f&~L.t&~L.f'1&~L.t'1&L.e'1&R.e'1&~R.f'1&~R.t'1
(hsesim)r0

You'll notice the ID's P0, P9, and P15. These refer to specific semicolons or
"places" in the hse. To see what these labels refer to, you may run the
following commands outside of the interactive simulation environment:

hsesim file.hse -g file.dot -l
dot -Tpng file.dot > file.png

The '-l' option adds labels to all of the places, transitions, and arcs. 
Now that you have set the current state to a reset state, you may take a look
at the current state.

(hsesim)t
~R.f&~R.t&L.e&R.e&~L.f&~L.t {
        (0) P0  L.e+ ; [R.e&L.f->...[]R.e&L.t->...]
}
~L.f'1&~L.t'1&L.e'1 {
        (1) P9  [L.e'1 -> [1->L.f'1+...[]1->L.t'1+...]
}
R.e'1&~R.f'1&~R.t'1 {
        (2) P15 R.e'1+ ; [R.f'1]
}

You can view the list of enabled transitions and fire one.

(hsesim)e
(0) T9.0:L.f'1+     (1) T10.0:L.t'1+
(hsesim)f1
0       T10.0   L.t'1+
(hsesim)e
(0) T2.0:[R.e&L.t]
(hsesim)f0
1       T2      [R.e&L.t]
(hsesim)e
(0) T3.0:R.t+
(hsesim)f0
2       T3.0    R.t+

You may also step through the simulation. This has two functions. As you
progress through te simulation, the simulator will remember all of the
transitions that have fired and in what order. If you decide to reset the
simulation after simulating and then step, this will re-execute the remembered
list of transitions. If you step through all of the remembered transitions,
it will continue to step randomly. This will execute the transitions in a 
*random order*. Unlike prsim, this simulator has no sense of time other than 
transition order.

(hsesim)e
(0) T9.0:L.f'1+     (1) T10.0:L.t'1+
(hsesim)f1
0       T10.0   L.t'1+
(hsesim)e
(0) T2.0:[R.e&L.t]
(hsesim)f0
1       T2      [R.e&L.t]
(hsesim)e
(0) T3.0:R.t+
(hsesim)f0
2       T3.0    R.t+
(hsesim)s6
3       T15     [R.t'1]
4       T4.0    L.e-
5       T16.0   R.e'1-
6       T11     [L.t'1&~L.e'1]
7       T13.0   L.t'1-
8       T12.0   L.f'1-                  [vacuous]
(hsesim)r0
(hsesim)s9
0       T10.0   L.t'1+
1       T2      [R.e&L.t]
2       T3.0    R.t+
3       T15     [R.t'1]
4       T4.0    L.e-
5       T16.0   R.e'1-
6       T11     [L.t'1&~L.e'1]
7       T13.0   L.t'1-
8       T12.0   L.f'1-                  [vacuous]

You can save this list of transitions to a file and load it up in a later 
simulation.

(hsesim)save test

(hsesim)load test
(hsesim)r0
(hsesim)s9
0       T10.0   L.t'1+
1       T2      [R.e&L.t]
2       T3.0    R.t+
3       T15     [R.t'1]
4       T4.0    L.e-
5       T16.0   R.e'1-
6       T11     [L.t'1&~L.e'1]
7       T13.0   L.t'1-
8       T12.0   L.f'1-                  [vacuous]

If you no longer want to simulate the future steps, you may clear them. This 
will remove any transitions in the list ahead of your current step. However,
transitions before your current step will still be remembered.

(hsesim)load test
(hsesim)r0
(hsesim)s9
0       T10.0   L.t'1+
1       T2      [R.e&L.t]
2       T3.0    R.t+
3       T15     [R.t'1]
4       T4.0    L.e-
5       T16.0   R.e'1-
6       T11     [L.t'1&~L.e'1]
7       T13.0   L.t'1-
8       T12.0   L.f'1-                  [vacuous]
(hsesim)r0
(hsesim)s3
0       T10.0   L.t'1+
1       T2      [R.e&L.t]
2       T3.0    R.t+
(hsesim)clear
(hsesim)s6
3       T15     [R.t'1]
4       T16.0   R.e'1-
5       T4.0    L.e-
6       T11     [L.t'1&~L.e'1]
7       T12.0   L.f'1-                  [vacuous]
8       T13.0   L.t'1-

=================================== Syntax ====================================

What is HSE? 
HSE stands for Handshaking Expansions. It is a step in between Communicating 
Hardware Processes (CHP) and Production Rules (PRs). Its a control flow 
language where all actions are limited to 1 bit boolean. There are only a few 
basic syntax structures most of which are composition operators. Spacing is
ignored during parsing. The following list explains what each syntax does.
Composition operators are listed by precedence from weakest to strongest.

-------------------------------------------------------------------------------

skip

This is just a no-op.

-------------------------------------------------------------------------------

x-
x+

Every variable in HSE represents a node in the circuit. "x-" sets the voltage
on that node to GND and "x+" sets the voltage on that node to VDD.

-------------------------------------------------------------------------------

P0 || P1 || ... || Pn

Parallel composition: do P0, P1, ..., and Pn in any interleaving.

-------------------------------------------------------------------------------

P0;P1;...;Pn

Sequential composition: do P0, then P1, then ..., then Pn.

-------------------------------------------------------------------------------

P0,P1,...,Pn

Internal parallel composition is the same as parallel composition.

-------------------------------------------------------------------------------

[ G0 -> P0
[] G1 -> P1
...
[] Gn -> Pn
]

[ G0 -> P0
: G1 -> P1
...
: Gn -> Pn
]

[G]

The selection composition represents choice. G0, G1, ..., Gn are called guards. 
They are boolean expressions that represent the condition of the selection and 
P0,P1,...,Pn are the processes that are executed for each condition.

A selection statement can either be deterministic as represented by the thick
bar operator '[]' or non-deterministic as represented by the thin bar operator 
':'. If it is a deterministic selection, then the guards are guaranteed by the
user to be mutually exclusive so only one can ever evaluate to true at any
given time. Meanwhile if it is non-deterministic, then an arbiter or in
some extreme cases, a synchronizer, must be used to guarantee the mutual
exclusion of the selection. Ultimately the selection operator implements the
following:

If G0 do P0, if G1 do P1, ... If Gn do Pn, else wait.

If there is no process specified as in the third example, then the process
is just a 'skip'. This is shorthand for a wait until operation, also
known simply as a 'guard'.

-------------------------------------------------------------------------------

*[ G0 -> P0
[] G1 -> P1
...
[] Gn -> Pn
]

*[ G0 -> P0
: G1 -> P1
...
: Gn -> Pn
]

*[P]

Repetitive selection behaves almost the same as the non-repetitive selection
statement. Think of it like a while loop.

While one of the guards (G0,G1,...,Gn) is true, execute the associated process
(P0,P1,...,Pn). Else, exit the loop.

If the guard is not specified, then the guard is assumed to be '1'. This
is shorthand for a loop that will never exit.

====================== Internal Representation of State =======================

The state of a node is represented by four basic values (-,0,1,X). '-' means
that the node is stable at either GND or VDD but the process doesn't know
which. '0' means the node is stable at GND. '1' means the node is stable at
VDD. And 'X' means the node is unstable or metastable (some unknown value
between GND and VDD).

(x+) drives the node 'x' to '1' and (x-) drives the node 'x' to '0'. If two 
assignments interfere as in (x+,x-), then the value of the node 'x' will
be driven to 'X'. If an assignment is unstable as in ([x];y+||x-), then the
node 'y' will be drive to 'X'.  

If there is a selection like ([1->x+:1->x-];y-), then at the semicolon before
y-, the value of the node 'x' will be '-'. (Yes I know this example does not
represent a real circuit because you don't know when the selection has
completed).

If a node has a value of 'X', then it will propagate as expected. For example
in (y-; [x]; y+) if the node 'x' is unstable, then after y+, the node 'y' will
also be unstable.

============================= Isochronic Regions ==============================

It has been shown that circuits that are entirely delay insensitive (make no
timing assumptions) aren't that useful. One of the weakest timing assumptions
you can make in order for the circuit class to be turing complete is called the
isochronic fork assumption. In order to implement this assumption, you can
identify isochronic regions. By default every reference to a node is assumed to 
be in the same isochronic region (region 0). However you may change the
isochronic region with the following syntax:

P'region

Where P is a process or node reference and 'region' is an integer representing
the name of the region. For example:

x'1+

(x+)'1

(x+,y+)'2

[  G0 -> P0
[] G1 -> P1
]'4

[x'1 & y'2]; z-

If there are two processes in two isochronic regions like (x-;y+;P || [y]; x+),
then during the process P, the value of the node 'x' will be '-' because the
process on the left knows that 'x' was '0' but that it will change to '1'. It
just doesn't know when. Meanwhil in the process on the right, the value of 'x'
will start at '-' and transition to '1' after the assignment x+.

========================== Non-Properly Nested HSE ============================

Note: Asynchronous circuits are ultimately sets of intertwined, highly parallel
sequences of events. The most basic way to visualize this is called a petri
net. Handshaking expansions are a way to represent that structure in a way that
is linearized in a human readable linguistic format. However, there are also
valid handshaking expansions that are not representable in a linguistic format.
These are called 'non-properly nested'. The tools have some support for these
types of HSE, however it is not entirely tested. They support an input format
for these HSE through the graphviz dot specification. To get an example of
this format, execute the following command on an hse:

hsesim file.hse -g file.dot

If you want to see what this graph looks like then execute this:

dot -Tpng file.dot > file.png

=========================== Special Environment ===============================

This only applies during a state space elaboration:

hsesim file.hse -eg file.dot

HSE can get very complex and it becomes very easy to represent circuits with
trillions of states. This means that it can start to take a long time to
explore the state space. In order to counter act this problem, there is support
for simulating multiple states within the environment simultaneously. Normally
when you specify processes, you compose them in parallel with a composition
operator like so (P0||P1||...||Pn). However, Lets say you want to treat the
processes Pi,Pi+1,...,Pn as special environment processes. Simply delete one
of the composition operators like so (P0||P1||...||Pi-1 Pi||Pi+1||...||Pn).
All of the processes Pi...Pn will be collapsed.

  No two special environment processes may communicate. No channel actions, no
  shared variables. A special environment process may communicate with a normal
  process.

  A special environment process may not be in the same isochronic region as a
  normal process.

  You will not receive any state information about the special environment
  processes.

=============================== Reset Behavior ================================

Because reset behavior can be a complex thing that has a multitude of timing
assumptions and different possible implementations, hsesim has a very basic
reset implementation. It goes as follows: as long as there isn't any choice to
be made, and we don't enter a loop, execute transitions and accumulate their
affect on the state into a reset state. Here is an example:

R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1

In this WCHB buffer, the reset state for each process is as follows:

Process 0: ~R.f&~R.t&L.e&R.e&~L.f&~L.t
Process 1: ~L.f&~L.t&L.e
Process 2: R.e&~R.f&~R.t

and the final hse after the reset behavior has been processed looks like this:

*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(*[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
*[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1

========================= Limited Non-Proper Nesting ==========================

In order to support things like initial token buffers where you reset the
circuit in the middle of the HSE, a limited reset-tagging system has been
added. 

R.f+,R.t-,L.e+,en+; [R.e&~L.f&~L.t];  
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; en-;
  (
     @ [~R.e]; R.f-,R.t- ||
     [~L.f & ~L.t]; L.e+ @
  ); en+
 ]

L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+]

In this system, the '@' symbol represents a reset token at the nearest
semicolon for current loop. So if there are multiple loops and you put a reset
token on the inner most loop, that loop will reset there on every iteration of
the outer loop.

================================== Examples ===================================

The following is a set of simple examples to get you started.

-------------------------------- WCHB Buffer ----------------------------------

R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1

-------------------------------- PCHB Split -----------------------------------

A.f-,A.t-,B.f-,B.t-,SL.e+; [~S.f & ~S.t & ~L.f & ~L.t & A.e & B.e];
*[
  ([  A.e & S.f & L.f -> A.f+
  [] A.e & S.f & L.t -> A.t+
  [] S.t -> skip
  ] ||
  [ B.e & S.t & L.f -> B.f+
  [] B.e & S.t & L.t -> B.t+
  [] S.f -> skip
  ]); SL.e-;
  (
    [~A.e | ~A.f & ~A.t -> A.f-, A.t-] ||
    [~B.e | ~B.f & ~B.t -> B.f-, B.t-]
  ); [~S.f & ~S.t & ~L.f & ~L.t -> SL.e+]
 ]||

(A.e+; [~A.f & ~A.t];
*[[A.t | A.f]; A.e-; [~A.t & ~A.f]; A.e+] ||

B.e+; [~B.f & ~B.t];
*[[B.t | B.f]; B.e-; [~B.t & ~B.f]; B.e+] ||

L.f-,L.t-; [SL.e];
*[[1 -> L.t+ : 1 -> L.f+]; [~SL.e]; (L.t-||L.f-); [SL.e]] ||

S.f-,S.t-; [SL.e];
*[[1 -> S.t+ : 1 -> S.f+]; [~SL.e]; (S.t-||S.f-); [SL.e]])'1

--------------------------------- PCHB Adder ----------------------------------

S.f-,S.t-,Co.f-,Co.t-,ABCi.e+; [S.e&Co.e&~A.f&~A.t&~B.f&~B.t&~Ci.f&~Ci.t];
*[
    (
        [   S.e & (A.t & B.f & Ci.f | A.f & B.t & Ci.f | A.f & B.f & Ci.t | A.t & B.t & Ci.t) -> S.t+
        []  S.e & (A.t & B.t & Ci.f | A.t & B.f & Ci.t | A.f & B.t & Ci.t | A.f & B.f & Ci.f) -> S.f+
        ] ||
        [   Co.e & (A.t & B.t & Ci.f | A.t & B.f & Ci.t | A.f & B.t & Ci.t | A.t & B.t & Ci.t) -> Co.t+
        []  Co.e & (A.t & B.f & Ci.f | A.f & B.t & Ci.f | A.f & B.f & Ci.t | A.f & B.f & Ci.f) -> Co.f+
        ]
    ); ABCi.e-;
    (
        [~S.e -> S.t-,S.f-] ||
        [~Co.e -> Co.t-,Co.f-]
    ); [~A.t & ~A.f & ~B.t & ~B.f & ~Ci.t & ~Ci.f];
    ABCi.e+
]

(S.e+; [~S.f&~S.t]; *[[S.t | S.f]; S.e-; [~S.t & ~S.f]; S.e+] ||

Co.e+; [~Co.f&~Co.t]; *[[Co.t | Co.f]; Co.e-; [~Co.t & ~Co.f]; Co.e+] ||

A.f-,A.t-; [ABCi.e];
*[[ 1 -> A.t+
  : 1 -> A.f+
  ]; [~ABCi.e]; A.t-,A.f-; [ABCi.e]
 ] ||

B.f-,B.t-; [ABCi.e];
*[[ 1 -> B.t+
  : 1 -> B.f+
  ]; [~ABCi.e]; B.t-,B.f-; [ABCi.e]
 ] ||

Ci.f-,Ci.t-; [ABCi.e];
*[[ 1 -> Ci.t+
  : 1 -> Ci.f+
  ]; [~ABCi.e]; Ci.t-,Ci.f-; [ABCi.e]
 ])'1

================================= Known Bugs ==================================

hsesim has not gone through full and rigorous testing. Take its feedback with a 
grain of salt. If you do find a bug, please send me the hse that causes it and 
a short description at eab242@cornell.edu. The following is a list of known
bugs.

-------------------------------------------------------------------------------

R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; ([~R.e&~L.f];R.f-||[~R.e&~L.t];R.t-); L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1

This is a possible implementation for a wchb buffer. Notice the following 
section:

([~R.e&~L.f];R.f-||[~R.e&~L.t];R.t-)

This section exercises a subtle but explosive bug. One of R.f- or R.t- are
vacuous. Suppose the non vacuous of those two transitions fires. This will 
allow R.e+ to fire, disabling the guard on the other branch. However, since the
transition it guards is vacuous, it incorrectly flags the guard as unstable.

The easiest way to get around this bug is to move the [~R.e] section of the 
guard to before the buggy section:

[~R.e];([~L.f];R.f-||[~L.t];R.t-)

Now the ~R.e guard completes before R.f- and R.t- are allowed to fire. This bug
is an artifact of treating guards as transitions with a completion time when
they really aren't. Because of the composable nature of guards, however, its
very difficult to fix this problem.

-------------------------------------------------------------------------------

