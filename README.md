it's only simple kernel for computer but it already have an gcc compiler inside so it can compile the C++ and C code 
to run the kernel you need to install qemu for booting 
for arch: sudo pacman -S qemu 
if you dont have an assembly language 
then install it with: sudo pacman -S nasm

also we need to install i686-elf-gcc 
for arch we need first install it from git repository 
command: git clone https://aur.archlinux.org/i686-elf-gcc.git
then need to get in that directory with: cd i686-elf-gcc 

then you'll see a file name PKGBUILD that's main file for unpacking i686-elf-gcc 
then if you try to type something to build you will get in trouble with user permission 
to solve it type: sudo chown -R $USER:$USER i686-elf-gcc    (or instead of i686-elf-gcc the name of the folder with PKGBUILD) it is giving permission for actual user to run it 

if you have already completed all that i listed 
run the command: makepkg -si 
(it will unpkg all that you're need for i686-elf-gcc)

okay you completed the full of the instruction 
you can easily run the kernel 
command: make run 
then there will be qemu cli with my grub you can whatever you want 

if i make it full of using 
that will be full guide to install it


