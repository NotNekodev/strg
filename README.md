# strg

A simple Wayland Compositor written in C using wlroots and uses lua for configuration

how 2 run

just run `./strg` in either an already running compositor (this will launch nested backend (X11 and Wayland)) or a TTY (will use KMS + DRM backend)

options:
- `-c`: which config file to load
- `-l`: which logfile to use

my run command btw
```bash
./strg -l strg.log -c example/config.lua
```
