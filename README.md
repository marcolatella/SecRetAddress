# SecRetAddress
SecRetAddress is a compiler plugin for the GNU C compiler (GCC) to protect Return Address or sensitive stack data from Buffer Overflow Attacks.

This is a part of a project done during my studies.

The idea behind the work was the study and analysis of the protection system Stack Canary, in order to determine its robustness and weaknesses in case of Buffer Overflow attacks or combined attacks exploiting multiple vulnerabilities.

## How it works

How said earlier, the purpose of this plugin is to protect C programs from Buffer Overflow attacks. In order to do this, the plugin parses, in compilation phase, each function of the source code to find arrays defined. If an array is found, the function will be instrumented by adding some protection code in the ENTRY point and in the EXIT point.
The protection code injected, allocates, when the function starts, a new memory, in which the return address is copied. When the function ends, the Ret Address stored in the stack is compared with the one copied in the previously created memory. if there is a mismatch, the program is terminated.


## Install GCC

The plugin was tested on an Ubuntu 20.04 machine.

First of all install GCC compiler:

``` 
sudo apt update
sudo apt install build-essential
```
Now check if API header files are correctly installed, by running this command:
```
gcc -print-file-name=plugin
```
If the command simply prints the word "plugin", it means that you have the wrong compiler.

Now install "gcc-plugin-dev" package for other dependencies.
My GCC version is 9.3.0, so I had to install version 9 of "gcc-plugin-dev".
Remember to change the version to that of your compiler.

```
sudo apt-get install -y gcc-9-plugin-dev
```

To check that the installation was successful you can follow this steps:
```
cd check_config
make
make check
```
If the commands print "[+] Plugin Initializated", that means that all went good. 

## Test Plugin

In test-plugin folder you can find a few file for testing the plugin functionalities. The "vuln.c" file is a very simple vulnerable progam, it is affect by Buffer Overflow and Format String bug vulnerabilities. 
Running these commands you should get the same output shown in the image below:
```
make
make protect
```
<img width="446" alt="Schermata 2021-04-30 alle 12 39 10" src="https://user-images.githubusercontent.com/82458106/116684276-1154ae80-a9b1-11eb-904f-0c70ea9eafb6.png">

Now the executable file is protected and no longer vulnerable!

## Usage




