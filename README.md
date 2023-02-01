# How to run

The registry modules are inside the "external_modules" folder.

The demo application to run the code is in the root folder.

RUN

```
BUILD_IN_DOCKER=1 make all term
```

inside a termainal that points to THIS folder.

The makefile expects to find a "RIOT" folder containing the RIOT OS source code next to its own folder.

Example filetree:

- source code (this folder)
- RIOT
