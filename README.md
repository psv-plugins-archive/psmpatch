# Download:
You can download v1.0 here: https://bitbucket.org/SilicaAndPina/psmpatch/downloads/PSMPatch.suprx  
It's worked well enough to change a song in Cytus Lambda as well as make a mod to app.exe to unlock every song XD  

# PSMPatch
Allows you to modify PSM Games, and also can be used to load PSM Homebrew.  
Simply put patched files into ux0:psm/TITLEID/Documents/p   
files here will be read instead of the files in /Application  
Think of it as PSM Repatch..

# How to decrypt PSM Games tho??!
Use [FuckPSSE:](https://bitbucket.org/SilicaAndPina/fuckpsse/src/master/README.md)  
to decrypt the PSSE Layer on PSM Games and then you can modify whatever you want!  
For executables, you can use dnSpy or ILLSpy to decompile and modify the executables.  

# About loading homebrew?
Well this is a bit of a hack-y system. but it should work.  
if you compile an app using the PSM SDK there is an unsigned copy of it created.  
you can simply copy the unsigned files into the patch folder and it'll load the homebrew instead of the original game!
PS: if u get compile errors while using the PSM SDK try running 'setx MSBUILDENABLEALLPROPERTYFUNCTIONS 1' in CMD.  

# Installation
Place the plugin under the \*ALL section of your config.txt and your good to go,  
*NOTE: Do not use this plugin at the same time as FuckPSSE. you will break the universe*  


