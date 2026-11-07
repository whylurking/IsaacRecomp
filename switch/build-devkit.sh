#!/bin/bash
# Builds from source everything needed to compile the Switch port (equivalent to installing
# devkitA64 + libnx + switch-sdl2 + switch-mesa with devkitPro's pacman, which is the recommended route if
# it's available to you: then "cmake -DCMAKE_TOOLCHAIN_FILE=cmake/switch.cmake" is all you need).
#
# Sources: binutils 2.46 and newlib 4.6.0.20260123 (Ubuntu archive), gcc 16.1.0 (github gcc-mirror),
# patches from devkitPro/buildscripts, switchbrew/libnx, switchbrew/switch-tools, devkitPro/general-tools,
# devkitPro/devkita64-rules, devkitPro/libdrm_nouveau v1.0.1, devkitPro/mesa (branch switch-20.1.0-rc3),
# libsdl-org/SDL release-2.28.5 + patch from devkitPro/pacman-packages.
# System dependencies (Ubuntu 24.04): build-essential libgmp-dev libmpfr-dev libmpc-dev libisl-dev
#   texinfo flex bison meson ninja-build python3-mako pkg-config liblz4-dev zlib1g-dev autoconf automake
set -e
SRC=${SRC:-$HOME/switch/src}          # sources already downloaded and patched (see above)
BLD=${BLD:-$HOME/switch/build}
export DEVKITPRO=/opt/devkitpro
export PATH=$DEVKITPRO/devkitA64/bin:$DEVKITPRO/tools/bin:$PATH
T=aarch64-none-elf; P=$DEVKITPRO/devkitA64
# System libraries built with -fPIC: .nro files are PIE executables. (devkitPro achieves this with
# a "pic" multilib; here everything is simply built that way.)
TF="-O2 -ffunction-sections -fdata-sections -fPIC"
J=-j$(nproc)

mkdir -p $BLD/binutils $BLD/gcc $BLD/newlib
(cd $BLD/binutils && $SRC/binutils-2.46.0/configure --prefix=$P --target=$T --disable-nls --disable-werror --disable-shared \
   --disable-debug --enable-lto --enable-plugins --enable-poison-system-directories --disable-gdb --disable-gprofng \
   --disable-gold --disable-sim && make $J && make install)
(cd $BLD/gcc && CFLAGS_FOR_TARGET="$TF" CXXFLAGS_FOR_TARGET="$TF" LDFLAGS_FOR_TARGET="" $SRC/gcc-16.1.0/configure \
   --target=$T --prefix=$P --enable-languages=c,c++ --with-gnu-as --with-gnu-ld --with-gcc --enable-cxx-flags='-ffunction-sections' \
   --disable-libstdcxx-verbose --enable-poison-system-directories --enable-threads=posix --disable-nls --disable-debug \
   --disable-libssp --disable-libgomp --disable-libstdcxx-pch --enable-libstdcxx-time=yes --enable-libstdcxx-filesystem-ts \
   --with-newlib=yes --with-native-system-header-dir=/include --with-sysroot=$P/$T --disable-tm-clone-registry \
   --disable-__cxa_atexit --with-march=armv8 --disable-multilib --with-pkgversion="devkitA64 (recomp)" --disable-bootstrap \
   && make $J all-gcc && make install-gcc)
(cd $BLD/newlib && CFLAGS_FOR_TARGET="$TF" $SRC/newlib-4.6.0.20260123/configure --disable-newlib-supplied-syscalls \
   --enable-newlib-mb --disable-newlib-wide-orient --enable-newlib-register-fini --target=$T --prefix=$P \
   && make $J && make install -j1)
(cd $BLD/gcc && make $J CFLAGS_FOR_TARGET="$TF" CXXFLAGS_FOR_TARGET="$TF" && make install-strip)
# gcc's limits.h must chain to newlib's (gcc was configured before newlib's headers existed)
G=$P/lib/gcc/$T/16.1.0/include/limits.h
cat $SRC/gcc-16.1.0/gcc/limitx.h $SRC/gcc-16.1.0/gcc/glimits.h $SRC/gcc-16.1.0/gcc/limity.h > $G

# devkitPro rules and tools
cp $SRC/devkita64-rules/base_rules $SRC/devkita64-rules/base_tools $P/
for t in general-tools switch-tools; do (cd $SRC/$t && ./autogen.sh && ./configure --prefix=$DEVKITPRO/tools && make && make install); done
# environment scripts (dkp-toolchain-vars / dkp-meson-scripts from pacman-packages) and portlibs pkg-config
cp $SRC/../pacman-packages/dkp-toolchain-vars/*.sh $SRC/../pacman-packages/dkp-meson-scripts/*.sh $DEVKITPRO/
mkdir -p $DEVKITPRO/portlibs/switch/bin
cp $SRC/../pacman-packages/switch/pkg-config/pkg-config.in $DEVKITPRO/portlibs/switch/bin/$T-pkg-config
chmod +x $DEVKITPRO/*.sh $DEVKITPRO/portlibs/switch/bin/$T-pkg-config

(cd $SRC/libnx && make $J && make install)
(cd $SRC/libdrm_nouveau && make $J && make install)
# mesa (nouveau, EGL, desktop OpenGL). Newlib already provides timespec_get.
# (its generators use ElementTree's getchildren(), gone since Python 3.9)
grep -rl 'getchildren()' --include=*.py $SRC/mesa-20.1.0-rc3 | xargs -r sed -i -E 's/([A-Za-z_]+)\.getchildren\(\)/list(\1)/g'
(cd $SRC/mesa-20.1.0-rc3 && $DEVKITPRO/meson-toolchain.sh switch > ../crossfile.txt && \
   sed -i "s/c_args = \['-D__SWITCH__',/c_args = ['-D__SWITCH__','-DHAVE_TIMESPEC_GET',/; s/cpp_args = \['-D__SWITCH__',/cpp_args = ['-D__SWITCH__','-DHAVE_TIMESPEC_GET',/" ../crossfile.txt && \
   meson setup --buildtype=plain --cross-file=../crossfile.txt --default-library=static --prefix=$DEVKITPRO/portlibs/switch \
     --libdir=lib build -Db_ndebug=true && ninja -C build && ninja -C build install && \
   sed -i "s,-lEGL,-lEGL -ldrm_nouveau," $DEVKITPRO/portlibs/switch/lib/pkgconfig/egl.pc)
(cd $SRC/SDL2-2.28.5 && source $DEVKITPRO/switchvars.sh && ./configure --prefix="$PORTLIBS_PREFIX" --host=$T --disable-shared \
   --enable-static --enable-audio --enable-joystick --disable-power --disable-filesystem --enable-cpuinfo --enable-pthreads \
   --enable-timers --enable-video && make $J && make install)
echo "devkitA64 + libnx + portlibs ready in $DEVKITPRO"
