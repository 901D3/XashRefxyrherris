# _**Building XashRefXyrherris**_

## Requirements (Windows)

[Xash3D Build instructions](https://github.com/FWGS/xash3d-fwgs#build-instructions)

## Building instructions

1. Clone this repo and place it in your Xash3D FWGS folder, assume Xash3D FWGS folder is `xash3d-fwgs/` \
   `git clone https://github.com/901D3/XashRefXyrherris/ xash3d-fwgs/3rdparty/`.
2. Setup GLEW, see `3rdparty/glew/README.md`.
3. *(Optional)* Configure wscript in XashRefXyrherris folder, if your target is to build software renderer, GL2.1/GLES2 renderer or whatever.
4. Add Subproject to SUBDIRS in wscript in `xash3d-fwgs/` to include XashRefXyrherris for building. \
   If you are truly lazy, use this `Subproject("3rdparty/XashRefXyrherris")`.
5. Build the project
