hsesim

===============================================================================

What is HSE? 
HSE stands for Handshaking Expansions. It is a step in between Communicating 
Hardware Processes (CHP) and Production Rules (PRs). Its a control flow 
language where all actions are limited to 1 bit boolean. There are only a few 
basic syntax structures most of which are composition operators. Spacing is
ignored during parsing. The following list explains what each syntax does.
Composition operators are listed by precedence from weakest to strongest.

-------------------------------------------------------------------------------

skip

This is just a no-op. It has been proven necessary to do certain operations.

-------------------------------------------------------------------------------

Assignment
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

===============================================================================

Note: Asynchronous circuits are ultimately sets of intertwined, highly parallel
sequences of events. The most basic way to visualize this is called a petri
net. Handshaking expansions are a way to represent that structure in a way that
is linearized in a human readable linguistic format. However, there are also
valid handshaking expansions that are not representable in a linguistic format.
These are called 'non-properly nested'. The tools have some support for these
types of HSE, however it is not entirely tested. They support an input format
for these HSE through the graphviz dot specification. 

