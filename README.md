# yalp
Yet another repository for LLVM passes

After many false starts and failed attempts, finally I am having a collection of out of tree llvm passes using the new pass manager. 
This is modeled after [llvm-tutor](https://github.com/banach-space/llvm-tutor).

## Build
To build. make sure llvm is installed, and in the root directory of this project, run 
```
# for linux
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR="/usr/lib/llvm-9/" ..
```
```
# for macOS
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR="/usr/local/opt/llvm@9/" ..
```
