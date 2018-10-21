# PSMPatch
Allows you to modify PSM Games, and also can be used to load PSM Homebrew.
Simply put patched files into ux0:psm/TITLEID/Documents/p 
files here will be read instead of the files in /Application

# How to decrypt PSM Games tho??!
Use FuckPSSE: https://bitbucket.org/SilicaAndPina/fuckpsse/src/master/README.md  
to decrypt the PSSE Layer on PSM Games and then you can modify whatever you want!  
For executables, you can use dnSpy or ILLSpy to decompile and modify the executables.  

# About loading homebrew?
Well this is a bit of a hack-y system. but it should work.  
if you compile an app using the PSM SDK there is an unsigned copy of it created.  
you can simply copy the unsigned files into the patch folder and it'll load the homebrew instead of the original game!  

