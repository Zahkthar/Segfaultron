# Segfaultron

A discord bot written in C with Concord library

To build the project, you need to build the concord library with these commands

```bash
# If you already installed concord
sudo make uninstall
make purge

make shared
sudo make install
```

Here we need the shared version of concord, since we have modules

## Modules

Segfaultron is made to be modular, you can build standalone Segfaultron modules that the bot will run at boot time.

The easiest way to create a new module is to clone the repo, duplicate a module in the module folder and build from here.

The built file, the .so, needs to be in the bin/modules/ folder to be read by the bot.

## TODO

