# HideFiles
Filter driver that hides files on NTFS / Windows operating systems. Created for educational purposes only, as a bachelor project for "System Software" discipline.

# How to run
For running this driver app, you can copy contents of /build folder to your computer with 32-bit Windows XP/Vista/7. The build comes with redistributables, so you should not worry about compatibility.

# How to build
For building the kernel driver, you should have Visual Studio 2019 (or newer) and WDK version 10.0 (or newer). Start blank WDM-template project and follow next steps:
1) Disable "Spectre Mitigation" in Configuration Properties -> C/C++ -> Code Generation.
2) Turn off "Treat warnings as errors" in Configuration Properties -> C/C++ -> General.
3) Delete .inf file from Resource Files.
4) Change "Entry Point" to "DriverEntry" in Configuration Properties -> Linker -> Advanced.
 
For building the user-mode app, you should have Visual Studio 2019 (or newer) and WindowsSDK version 10.0 (or newer). Start blank project and follow next steps:
1) If you want to build just the GUI, skip this step. If you want to build console app instead of GUI, change "EntryPoint" to "main_console_entry_point" in Configuration Properties -> Linker -> Advanced.
2) Turn on "Copy C++ Runtime to OutDir" in Configuration Properties -> Advanced.
