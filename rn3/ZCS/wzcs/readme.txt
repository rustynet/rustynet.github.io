		Windows ZCS Device Switcher
		by leshiy[cin!],
		Idea & Design by ze_D0g[cin!].

Current Version: 1.0
--------------------------
Features And Usage:

WZCS sends a value of 0xFF to LPT port when switching to "On" state, 
and a value of 0x00 when switching to "Off" state.

Left-click on  tray icon changes current state of WZCS.
You can explicitly set or change the state of switcher from command line:

>wzcs.exe on

and 

>wzcs.exe off 

or by sending Windows Message to WZCS window from your program:

SendMessage(FindWindow("WZCSWindow",NULL),WM_COMMAND,9070,0); // switches to "on"
SendMessage(FindWindow("WZCSWindow",NULL),WM_COMMAND,9071,0); // switches to "off"

-------------------------
WZCS PRO additional features:
 - LPT port address selection (one of 0x378, 0x278 or 0x3BC);
 - separate bits switching;
 - using bit mask for bits switching (selected bits are switched by on/off commands);
 
-------------------------
Software used:

MinGW http://www.mingw.org,
Dev-C++ by Bloodshed Software http://www.bloodshed.net, 
Inpout32 library by Logix4u http://www.logix4u.net.
-------------------------

mailto:leshiy@bigmir.net.
11.01.2004 20:05 (v0.1)
18.01.2004 17:28 (v0.2)
25.01.2004 18:16 (v1.0)