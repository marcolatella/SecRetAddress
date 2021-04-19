# SecRetAddress
SecRetAddress is a compiler plugin for the GNU C compiler (GCC) to protect Return Address or sensitive stack data from Buffer Overflow Attacks.

This is a part of the a project done during my studies.

The idea behind the work was the study and analysis of the protection system Stack Canary, in order to determine its robustness and weaknesses in case of Buffer Overflow attacks or combined attacks exploiting multiple vulnerabilities.




## Install GCC

The plugin was tested on an Ubuntu 20.04 machine

``` 
sudo apt update
sudo apt install build-essential
```
Now check if API header files are installed correctly, running this command:
```
gcc -print-file-name=plugin
```
If the command simply prints the word "plugin", that means you have the wrong compiler.

Now install "gcc-plugin-dev" package for other dependencies.
My GCC version is 9.3.0, so I had to install version 9 of "gcc-plugin-dev"

```
sudo apt-get install -y gcc-9-plugin-dev
```

