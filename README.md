# um_tgen - Traffic generator

Traffic generator for Ultra Messaging.

# Table of contents

- [um_tgen - Traffic generator](#um_tgen---traffic-generator)
- [Table of contents](#table-of-contents)
- [Introduction](#introduction)
  - [Repository](#repository)
- [Variables and Looping](#variables-and-looping)
  - [Special Variables](#special-variables)
- [REPL](#repl)
- [Instruction Set](#instruction-set)
  - [Comment](#comment)
  - [Sendt](#sendt)
  - [Sendc](#sendc)
  - [Set](#set)
  - [Label](#label)
  - [Loop](#loop)
  - [Delay](#delay)
  - [Repl](#repl)
- [TODO](#todo)
- [COPYRIGHT AND LICENSE](#copyright-and-license)

<sup>(table of contents from https://luciopaiva.com/markdown-toc/)</sup>

# Introduction

This traffic generator leverages the "tgen" module.
I.e. "um_tgen.c" code knows how to talk UM,
and the "tgen.c" code orchestrates the traffic generation.
See https://github.com/fordsfords/tgen.

Here's an example usage:
````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "
  delay 200 msec # let topic resolution happen.
  sendc 700 bytes 2 persec 10 msgs
  delay 2 sec   # linger to allow NAK/retransmits to complete."
````
Note that the "-s" option (script) has a multi-line value.
This same script could be written without comments and with semi-colons instead of newlines:
````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "delay 200 msec; sendc 700 bytes 2 persec 10 msgs; delay 2 sec"
````

## Repository

See https://github.com/UltraMessaging/um_tgen for code and documentation.

# Variables and Looping

The tgen package supports 26 general-purpose integer variables ('a' - 'z').
They are most commonly used with the "loop" instruction,
or to communicate with the application as "special variables" (see below).

Here's an example of a loop instruction:
````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "
  delay 200 msec # let topic resolution happen.
  set i 10
  label a
    sendt 700 bytes 50 kpersec 4 sec
    sendt 700 bytes 999 mpersec 100 msec
  loop a i  # Decrement i, loop to a while i > 0.
  delay 4 sec   # linger to allow NAK/retransmits to complete."
````
The two "sendt" instructions send 4 seconds at 50,000 msgs/sec followed by a 100 ms burst at maximum send rate.
This is repeated 10 times.

Note that the label ('a' - 'z') are a separate name space from variable ('a' - 'z').
I.e. you can have a label 'a' and an unrelated variable 'a'.

## Special Variables

Although the tgen package does not assign any special meaning to any variable,
the application using tgen can assign meaning according to its needs.

The um_tgen application assigns special meaning to the following variables:
* **'l'** - Loss percentage for LBT-RM. Valid from 0 to 100.

Here's an example of using the 'l' special variable:
````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "
  delay 200 msec # let topic resolution happen.
  sentc 700 bytes 1 persec 1 msgs  # Send one message.
  set l 100  # Set 100% loss.
  sentc 700 bytes 1 persec 1 msgs  # This message is lost.
  set l 0  # Set 0% loss.
  delay 1 sec  # Let next session message trigger repair."
````

# REPL

A [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop)
is a "Read-Eval-Print Loop",
allowing the user to interactively enter commands.

Here's an example of using the 'repl' instruction:

````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "repl"
repl?
````
The "um_tgen" command has created the context and the source,
and is now waiting for you to enter a command.
For example:
````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "repl"
repl? sentc 700 bytes 1 persec 1 msgs
repl?
````
This sent one message.

To exit the REPL loop and delete the source and context,
enter "control-d" (signals EOF on standard input).

However, note that the REPL is purely interactive,
so the "label" and "loop" instructions don't work.

# Instruction Set

## Comment
````
[instruction] # comment
````

Example:
````
# comment 1
delay 1 sec # comment 2
````

## Sendt

Send a set of messages at a requested rate for a specified
period of time.
````
sendt N {bytes|kbytes|mbytes} R {persec|kpersec|mpersec} T {sec|msec|usec}
````
where:
* N - size of message ('kbytes' = 1,000 bytes, 'mbytes' = 1,000,000 bytes).
* R - send rate ('kpersec' = 1,000 per sec, 'mpersec' = 1,000,000 per sec).
* T - time sending ('msec' = milliseconds, 'usec' = microseconds).

Example:
````
sendt 10 kbytes 30 kpersec 200 msec
````
Send messages of 10,000 bytes at a rate of 30,000 messages/sec for 200 milliseconds.

## Sendc

Send a set of messages at a requested rate for a specified
number of messages.
````
sendc N {bytes|kbytes|mbytes} R {persec|kpersec|mpersec} C {msgs|kmsgs|mmsgs}
````
where:
* N - size of message ('kbytes' = 1,000 bytes, 'mbytes' = 1,000,000 bytes).
* R - send rate ('kpersec' = 1,000 per sec, 'mpersec' = 1,000,000 per sec).
* C - message count ('kmsgs' = 1,000 messages, 'mmsgs' = 1,000,000 messages)).

Example:
````
sendc 10 kbytes 30 kpersec 200 kmsgs
````
Send messages of 10,000 bytes at a rate of 30,000 messages/sec for 200,000 messages.

## Set

Set a variable to a value.
````
set ID VAL
````
where:
* ID - variable identifier 'a' .. 'z' (26 variables total).
* VAL - numeric integer value.

Example:
````
set b 256
set c 0x1F
````
Sets variable b to 256 and varialbe c to 31.

## Label

Define a label in the script.
````
label ID
````
where:
* ID - label identifier 'a' .. 'z' (26 labels total).

Example:
````
label b
````

## Loop

Decrement a varialbe and loop if positive.
````
loop LAB_ID VAR_ID
````
where:
* LAB_ID - label to branch to 'a' .. 'z'.
* VAR_ID - variable to decrement.

Example:
````
set i 5
label a
  set j 3
  label b
    delay 1 msec
  loop b j
loop a i
````
Nested loop with delay executed 15 times.

## Delay

Pause for a period of time.
Uses busy looping to get high accuracy.
````
delay T {sec|msec|usec}
````
where:
* T - time sleeping ('msec' = milliseconds, 'usec' = microseconds).

Example:
````
delay 10 msec
````

## Repl

Enter Read-Eval-Print Loop.
Read instructions from stdin and
execute them until EOF.
````
repl
````

Example:
````
repl
````
Usually used interactively,
with EOF supplied by typing
<control-d>.

# TODO

I want to be careful not to bloat this tool.
Sometimes it's better to clone a tool for a new
application.

* It might be nice to support setting the contents
of the messages being sent.
It should probably allow inclusion of a changing
value with each message.
Possibly even "verifiable" messages (per the
UM example apps).

* It might be nice to support multiple sources.

* It might be nice to support file inclusion for scripts.

* It might be nice to supply instruction arguments via
variables.
I.e. instead of "delay 5 msec", maybe something like "delay i msec"
which uses the value in variable 'i'.

* It might be nice to support multi-character variable names
and labels.
Allows better self-documenting scripts.

* It might be nice to support functions, parameters,
arrays, strings, floating point variables, macros,
structures, etc, etc, etc.

**OR NOT**

It probably doesn't make sense to do many (if any) of those.
Instead, a user should just write their script in C,
calling the "..._run" functions as needed.
That gives you the full richness and speed of C.

It's hard to justify putting too much effort into
making a "little language" rich.
Olin Shivers makes a good case against in the introductory sections of
https://3e8.org/pub/scheme/doc/Universal%20Scripting%20Framework%20(Lambda%20as%20little%20language).pdf

# COPYRIGHT AND LICENSE

All of the documentation and software included in this and any
other Informatica Ultra Messaging GitHub repository
Copyright (C) Informatica. All rights reserved.

Permission is granted to licensees to use
or alter this software for any purpose, including commercial applications,
according to the terms laid out in the Software License Agreement.

This source code example is provided by Informatica for educational
and evaluation purposes only.

THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR
PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE
UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
BE LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR
INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE
TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF
THE LIKELIHOOD OF SUCH DAMAGES.
