#!/bin/bash

meson setup --wipe budir
meson compile -v -C budir
