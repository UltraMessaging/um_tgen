# um_tgen - Traffic generator

Traffic generator for Ultra Messaging.

# Table of contents

<!-- mdtoc-start -->
&bull; [um_tgen - Traffic generator](#um_tgen---traffic-generator)  
&bull; [Table of contents](#table-of-contents)  
&bull; [Introduction](#introduction)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Repository](#repository)  
&bull; [Quick Start](#quick-start)  
&bull; [Scripting Language](#scripting-language)  
&bull; [Sending Messages](#sending-messages)  
&bull; [Smart Sources](#smart-sources)  
&bull; [Variables, Labels, and Looping](#variables-labels-and-looping)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Special Variable for Loss](#special-variable-for-loss)  
&bull; [REPL](#repl)  
&bull; [Usage Help](#usage-help)  
&bull; [Instruction Set](#instruction-set)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Comment](#comment)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Sendt](#sendt)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Sendc](#sendc)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Set](#set)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Label](#label)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Loop](#loop)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Delay](#delay)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Repl](#repl)  
&bull; [TODO](#todo)  
&bull; [COPYRIGHT AND LICENSE](#copyright-and-license)  
<!-- TOC created by '../mdtoc/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->

# Introduction

This traffic generator leverages the "tgen" module.
I.e. "um_tgen.c" code knows how to talk UM,
and the "tgen.c" code orchestrates the traffic generation.
See https://github.com/fordsfords/tgen.

Note that this tool does not have a fancy GUI.
It is mostly intended for Unix command-line use,
although it also has a limited interactive mode
(also Unix command-line based).

## Repository

See https://github.com/UltraMessaging/um_tgen for code and documentation.

# Quick Start

To build the tool, download the source files from GitHub.
Then:

1. Log into Linux system with Ultra Messaging installed.
1. Download the um_tgen repository.
1. Copy "lbm.sh.example" to "lbm.sh".
Edit "lbm.sh" and insert your license key and UM installation path.
1. Run the "tst.sh" script.
This builds and runs a very small "smoke test" of tool.

# Scripting Language

The um_tgen tool uses the "little language" implemented by the "tgen" package.
The scripts written in that language are very simple.

The format of a script is a series of instructions separated by
either newlines or semi-colons.
Fields within an instruction are separated by one or more
whitespace (spaces and tabs are considered equivelant).
Indention is allowed.

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

Each instruction consists of a keyword,
followed by zero or more values and keywords.
The field format and ordering for a given instruction is fixed.
For example, this is a valid "sendt" instruction:
````
sendt 700 bytes 50 kpersec 3 sec
````
These are invalid:
````
sendt 50 kpersec 700 bytes 3 sec  # Field order is wrong.
sendt 700 bytes 50 kpersec        # 2 fields missing at end.
SendT 700 Bytes 50 KPerSec 3 Sec  # Upper-case not allowed.
````

Numeric fields may be specified in hexidecimal by prefixing
with "0x".

# Sending Messages

Sending messages is the basic function of a traffic generator.
The um_tgen tool has two instructions:
* sendt - send messages for a specified period of time.
* sendc - send a specific number of messages.

Both allow specifying a sending rate.
It uses a busy-looping algorithm to achieve
even spacing between messages.
For example:
````
sendt 700 bytes 50 kpersec 3 sec
````
This will send 700-byte messages at very close to
50,000 messages per second for 3 seconds.
You will see that the messages are separated by
almost exactly 20 microseconds.

Note that the tool does not initialize the
message contents.
It's just whatever malloc returned.

# Smart Sources

Starting with UM version 6.10,
a fast, low-jitter form of UM source was introduced,
called [Smart Source](https://ultramessaging.github.io/currdoc/doc/Design/advancedoptimizations.html#smartsources).
Since we want the traffic generator to have the highest possible rate,
um_tgen uses smart sources by default.
You can override and force um_tgen to use generic sources with the "-g" command-line
option.

To build this tool with a version of UM prior to 6.10,
omit the "-DUM_SSRC" from the build command in "tst.sh".
This omits the code for Smart Sources.

# Variables, Labels, and Looping

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

## Special Variable for Loss

Although the tgen package does not assign any special meaning to any variable,
the application using tgen can assign meaning according to its needs.

The um_tgen application assigns special meaning to the following variables:
* **'l'** - Loss percentage for LBT-RM. Valid from 0 to 100.

Here's an example of using the 'l' special variable:
````
./um_tgen -a 2 -g -x um.xml -t topic1 -s "
  delay 200 msec # let topic resolution happen.
  sendc 700 bytes 1 persec 1 msgs  # Send one message.
  set l 100  # Set 100% loss.
  sendc 700 bytes 1 persec 1 msgs  # This message is lost.
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
For example, type a "sendc" command to send a message:
````
repl? sendc 700 bytes 1 persec 1 msgs
repl?
````

To exit the REPL loop, type "ctrl-d" (signals EOF on standard input).
This continues the script to the next instruction.

However, note that the REPL is purely interactive.
The "label" and "loop" instructions don't work.

# Usage Help

````
Usage: um_tgen [-h] [-a affinity_cpu] [-c config] [-g] [-p persist_mode] -s script_string [-x xml_config]

where:
  -h : print help
  -a affinity_cpu : bitmap for CPU affinity for send thread [-1]
  -c config : configuration file; can be repeated []
  -f flags : flags to pass to tgen_create() [0]
  -g : generic source [0]
  -p ''|r|s : persist mode (empty=streaming, r=RPP, s=SPP) []
  -s 'script' : test script (required)
  -x xml_config : XML configuration file []
````

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
Send messages of 10,000 bytes each at a rate of 30,000 messages/sec for 200 milliseconds.

Note that the tool does not initialize the
message contents.
It's just whatever malloc returned.

Note that sendt will make its best effort to send at the requested rate.
If you specify a rate that UM cannot support (like 999 mpersec),
it will simply send without any delay between sends.

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
Send messages of 10,000 bytes each at a rate of 30,000 messages/sec for 200,000 messages.

Note that the tool does not initialize the
message contents.
It's just whatever malloc returned.

Note that sendc will make its best effort to send at the requested rate.
If you specify a rate that UM cannot support (like 999 mpersec),
it will simply send without any delay between sends.

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
Sets variable b to 256 and variable c to 31.

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

Decrement a variable and loop if positive.
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
Nested loops. The delay is executed 5 * 3 = 15 times.

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
with EOF supplied by typing "ctrl-d".

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

* It might be nice to support multiple sources (like lbmmsrc or lbmstrm).

* It might be nice to support file inclusion for scripts.

* It might be nice to supply instruction arguments via
variables.
I.e. instead of "delay 5 msec", maybe something like "delay i msec"
which uses the value in variable 'i'.

* It might be nice to support multi-character variable names
and labels.
Allows better self-documenting scripts.

* It might be nice to be able to orchestrate multiple publishers
on different hosts.
E.g. be able to burst multiple publishers at the same time.

* It might be nice to support if/then/else, for loops, while loops, functions, parameters,
arrays, strings, floating point variables, macros, structures, etc, etc, etc.

**OR NOT**

It probably doesn't make sense to do many (if any) of those.
Instead, a user should just write their script in C,
calling the "..._run" functions as needed.
That gives you the full richness and speed of C.

It's hard to justify putting too much effort into
making a "little language" rich.
Olin Shivers makes a good case against it in the introductory sections of
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
