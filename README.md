Simple Kernel Guide

This is a simple kernel for computer, but it already has a GCC compiler inside, so it can compile C and C++ code. To run the kernel, you need to install QEMU for booting.

Step 1: Install QEMU

For Arch Linux:

sudo pacman -S qemu

Step 2: Install NASM (if you don't have an assembler)

sudo pacman -S nasm

Step 3: Install i686-elf-gcc

For Arch, you need to install it from the git repository:

git clone https://aur.archlinux.org/i686-elf-gcc.git
cd i686-elf-gcc

You will see a file named PKGBUILD, which is the main file for building i686-elf-gcc. If you try to build it now, you may get a permission error. To fix it:

sudo chown -R $USER:$USER i686-elf-gcc

Then build and install the package:

makepkg -si

Step 4: Run the kernel

Once everything is installed, you can easily run the kernel:

make run

This will start QEMU CLI with GRUB, and you can do whatever you want inside.


---

✅ Note: All commands inside the triple-backtick blocks are copyable on GitHub — the user can click the “Copy” button in the top-right corner of the block.