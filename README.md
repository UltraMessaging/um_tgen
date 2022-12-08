# um_tgen - Traffic generator

Traffic generator for Ultra Messaging.

# Table of contents

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

## COPYRIGHT AND LICENSE

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

## Repository

See https://github.com/UltraMessaging/um_tgen for code and documentation.
