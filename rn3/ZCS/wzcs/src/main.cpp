#include <windows.h>
#include <shellapi.h>

#include "main.h"
#define ST_OFF 0
#define ST_ON 1
/* prototype (function typedef) for DLL function Inp32: */

typedef short _stdcall (*inpfuncPtr)(short portaddr);
typedef void _stdcall (*oupfuncPtr)(short portaddr, short datum); 

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
char szClassName[ ] = "WZCSWindow";

HINSTANCE hSelf;
HINSTANCE hLib;
inpfuncPtr inp32;
oupfuncPtr oup32; 

bool pwr_on=false;  //program main state
bool bits_state[8]; // bits state
bool bits_default[8];// which bits are default
short port=0x378;

#define WM_TRAY_CLICK WM_USER+1

int LoadIOLib()
{
     short x;
     int i;

     /* Load the library */
     hLib = LoadLibrary("inpout32.dll");

     if (hLib == NULL) {
          MessageBox(NULL,"LoadLibrary Failed.\n","Error",0);
          return -1;
     }

     /* get the address of the function */

     inp32 = (inpfuncPtr) GetProcAddress(hLib, "Inp32");

     if (inp32 == NULL) {
          MessageBox(NULL,"GetProcAddress for Inp32 Failed.\n","Error", 0);
          return -1;
     }


     oup32 = (oupfuncPtr) GetProcAddress(hLib, "Out32");

     if (oup32 == NULL) {
          MessageBox(NULL,"GetProcAddress for Oup32  Failed.\n","Error",0);
          return -1;
     }     
}
void UnLoadIOLib()
{
     FreeLibrary(hLib); 
     oup32=NULL;
     inp32=NULL;
}

void WriteLPT(short ax)
{
    if (oup32) (oup32)(port,ax);
}
void AddTrayIcon(HWND hWnd)
{
       NOTIFYICONDATA id;  
       ZeroMemory(&id,sizeof(id));   
       
       char szTip[] ="Power";
       
       id.cbSize=sizeof(id);
       id.hWnd=hWnd;
       id.uID=0;
       id.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP	;
       id.uCallbackMessage=WM_TRAY_CLICK;
       id.hIcon=LoadIcon (hSelf, "MAINICON");
       CopyMemory(&(id.szTip[0]),&szTip[0],sizeof(szTip));
       Shell_NotifyIcon(
            NIM_ADD, // message identifier
            &id // pointer to structure
       );
}           
                        
void DeleteTrayIcon(HWND hWnd)
{
       NOTIFYICONDATA id;  
       ZeroMemory(&id,sizeof(id));   
       
       char szTip[] ="Power";
       
       id.cbSize=sizeof(id);
       id.hWnd=hWnd;
       id.uID=0;
       id.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP	;
       id.uCallbackMessage=WM_TRAY_CLICK;
//       id.hIcon=LoadIcon (hSelf, "MAINICON");
       CopyMemory(&(id.szTip[0]),&szTip[0],sizeof(szTip));
       Shell_NotifyIcon(
            NIM_DELETE, // message identifier
            &id // pointer to structure
       );
}           

void ChangeTrayIcon(HWND hWnd,char* IconName)
{
       NOTIFYICONDATA id;  
       ZeroMemory(&id,sizeof(id));   
       
       char szTip[] ="Power";
       
       id.cbSize=sizeof(id);
       id.hWnd=hWnd;
       id.uID=0;
       id.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP	;
       id.uCallbackMessage=WM_TRAY_CLICK;
       id.hIcon=LoadIcon (hSelf, IconName);
       CopyMemory(&(id.szTip[0]),&szTip[0],sizeof(szTip));
       Shell_NotifyIcon(
            NIM_MODIFY, // message identifier
            &id // pointer to structure
       );
}           


void ShowState(HWND hWnd,bool st_on)                       
{
    char szOnIcon[]="ONICON";
    char szOffIcon[]="OFFICON";
    
    HMENU hmenu=GetMenu(hWnd);
    HMENU hmenuPopup=GetSubMenu(hmenu, 0); 
 
    if (hmenuPopup) {
       if (st_on) 
          {
             CheckMenuRadioItem(hmenuPopup,CM_POWER_ON,CM_POWER_OFF,CM_POWER_ON,MF_BYCOMMAND);
             ChangeTrayIcon(hWnd,&szOnIcon[0]);
          }           
       else
          {   
             CheckMenuRadioItem(hmenuPopup,CM_POWER_ON,CM_POWER_OFF,CM_POWER_OFF,MF_BYCOMMAND);
             ChangeTrayIcon(hWnd,&szOffIcon[0]);
          }   
       #ifdef PRO
             UINT uCheck; 
             for (int i=0;i<8;i++) {
                 uCheck=(bits_default[i])?(MF_CHECKED):(MF_UNCHECKED);
                 CheckMenuItem(hmenuPopup,CM_BIT+i,MF_BYCOMMAND | uCheck);                        
                 uCheck=(bits_state[i])?(MF_CHECKED):(MF_UNCHECKED);
                 CheckMenuItem(hmenuPopup,CM_DBIT+i,MF_BYCOMMAND | uCheck);                        
             }    
             switch (port) {            
                 case 0x378: 
                      CheckMenuRadioItem(hmenuPopup,CM_ADD_378,CM_ADD_3BC,CM_ADD_378,MF_BYCOMMAND);
                 break;     
                 case 0x278: 
                      CheckMenuRadioItem(hmenuPopup,CM_ADD_378,CM_ADD_3BC,CM_ADD_278,MF_BYCOMMAND);
                 break;     
                 case 0x3BC: 
                      CheckMenuRadioItem(hmenuPopup,CM_ADD_378,CM_ADD_3BC,CM_ADD_3BC,MF_BYCOMMAND);
                 break;     
             };
       #endif   
    }
}

void SetPwrState(bool apwr_on)
{
    pwr_on=apwr_on;
    for (int i=0;i<8;i++) if (bits_default[i]) bits_state[i]=pwr_on;
}
void SwitchPower()
{
    short port_state=0;
    for (int i=0; i<8; i++){
        port_state=port_state+(bits_state[i] << i);
    }
    WriteLPT(port_state);   
}

bool  AntiDoubleLoad()
{
    CreateMutex(NULL, true , "WZCS AntiDoubleLoader");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
             return 1;
    }
    else  return 0;
    
}

void LoadParams()
{
    HKEY hk; 
    DWORD dwData; 
    DWORD bufSize; 
    DWORD bufType;
    char szBuf[80]; 

    if (RegCreateKey(HKEY_CURRENT_USER,
            "Software\\cyberpunk.net.ua\\WZCS", &hk)) return; 
 
    bufSize=sizeof(DWORD);
    bufType=REG_DWORD;
    if (RegQueryValueEx(
            hk,             // subkey handle 
            "pwr on",       // value name 
            0,                        // must be zero 
            &bufType,            // value type 
            (BYTE*)(&dwData),           // pointer to value data 
            &bufSize
            )==ERROR_SUCCESS) pwr_on=dwData;      

    bufSize=sizeof(DWORD);
    bufType=REG_DWORD;
 
    if (RegQueryValueEx(
            hk,             // subkey handle 
            "port",       // value name 
            0,                        // must be zero 
            &bufType,            // value type 
            (BYTE*)(&dwData),           // pointer to value data 
            &bufSize
            )==ERROR_SUCCESS) port=dwData;

    for (int i=0; i<8; i++) {
        dwData=bits_state[i];
        ZeroMemory (&szBuf,80);
        strcpy((char*)szBuf, "state_"); 
        szBuf[6]=i+0x30;
        bufSize=sizeof(DWORD);
        bufType=REG_DWORD;

        if (RegQueryValueEx(
                hk,             // subkey handle 
                szBuf,       // value name 
                0,                        // must be zero 
                &bufType,            // value type 
                (BYTE*)(&dwData),           // pointer to value data 
                &bufSize
                )==ERROR_SUCCESS) bits_state[i]=dwData;
       
    }

    for (int i=0; i<8; i++) {
        dwData=bits_default[i];
        ZeroMemory (&szBuf,80);
        strcpy((char*)szBuf, "mask_"); 
        szBuf[5]=i+0x30;
        bufSize=sizeof(DWORD);
        bufType=REG_DWORD;

        if (RegQueryValueEx(
                hk,             // subkey handle 
                szBuf,       // value name 
                0,                        // must be zero 
                &bufType,            // value type 
                (BYTE*)(&dwData),           // pointer to value data 
                &bufSize
                )==ERROR_SUCCESS) bits_default[i]=dwData;
       
    }
    RegCloseKey(hk); 
}

void SaveParams()
{
    HKEY hk; 
    DWORD dwData; 
    char szBuf[80]; 

    if (RegCreateKey(HKEY_CURRENT_USER,
            "Software\\cyberpunk.net.ua\\WZCS", &hk)) return; 
 
    dwData=pwr_on;
    RegSetValueEx(
            hk,             // subkey handle 
            "pwr on",       // value name 
            0,                        // must be zero 
            REG_DWORD,            // value type 
            (BYTE*)(&dwData),           // pointer to value data 
            sizeof(DWORD)
            );       // length of value data 
 

    dwData=port;
    RegSetValueEx(
            hk,             // subkey handle 
            "port",       // value name 
            0,                        // must be zero 
            REG_DWORD,            // value type 
            (BYTE*)(&dwData),           // pointer to value data 
            sizeof(DWORD)
            );       // length of value data 

    for (int i=0; i<8; i++) {
        dwData=bits_state[i];
        ZeroMemory (&szBuf,80);
        strcpy((char*)szBuf, "state_"); 
        szBuf[6]=i+0x30;
        RegSetValueEx(
                hk,             // subkey handle 
                szBuf,       // value name 
                0,                        // must be zero 
                REG_DWORD,            // value type 
                (BYTE*)(&dwData),           // pointer to value data 
                sizeof(DWORD)
                );       // length of value data 
       
    }

    for (int i=0; i<8; i++) {
        dwData=bits_default[i];
        ZeroMemory (&szBuf,80);
        strcpy((char*)szBuf, "mask_"); 
        szBuf[5]=i+0x30;
        RegSetValueEx(
                hk,             // subkey handle 
                szBuf,       // value name 
                0,                        // must be zero 
                REG_DWORD,            // value type 
                (BYTE*)(&dwData),           // pointer to value data 
                sizeof(DWORD)
                );       // length of value data 
       
    }
    RegCloseKey(hk); 
}


int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
    char szStrON[]="ON";
    char szStrOFF[]="OFF";
    bool bChange=false;

    if (lpszArgument) {
        lpszArgument=strupr(lpszArgument);
        if (strstr(lpszArgument,&szStrON[0])) {
                pwr_on=true;
                bChange=true;
        }        
        if (strstr(lpszArgument,&szStrOFF[0])) {
                pwr_on=false;
                bChange=true;
        }        

    }
          
    if (AntiDoubleLoad()) {
        if (bChange) {
            if (pwr_on) SendMessage(FindWindow("WZCSWindow",NULL),WM_COMMAND,CM_POWER_ON,0);
            else SendMessage(FindWindow("WZCSWindow",NULL),WM_COMMAND,CM_POWER_OFF,0);        
        }
        return 1;
    } 



    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    hSelf=hThisInstance;
    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (hThisInstance, "MAINICON");
    wincl.hIconSm = LoadIcon (hThisInstance, "MAINICON");
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = "MAINMENU";                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_HIGHLIGHT;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "WZCS",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );


    /* Make the window visible on the screen */
//    ShowWindow (hwnd, nFunsterStil);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
             LoadIOLib();
             
             //init starting state
             for (int i=0; i<8; i++) bits_state[i]=0; // bits state
             for (int i=0; i<8; i++) bits_default[i]=1;// which bits are switched by default default
             port=0x378;

             #ifdef PRO
                LoadParams();
             #endif 
             AddTrayIcon(hwnd);
             SetPwrState(pwr_on);
             SwitchPower();
             ShowState(hwnd,pwr_on);                                   
             break;
      case WM_COMMAND:
         switch(LOWORD(wParam))
         {
            case CM_POWER_ON:
                SetPwrState(true);
                SwitchPower();
                ShowState(hwnd,pwr_on);                                   
            break;
            case CM_POWER_OFF:
                SetPwrState(false);
                SwitchPower();
                ShowState(hwnd,pwr_on);                                   
            break;
            case CM_EXIT:
               PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
            case CM_ABOUT:           
               #ifdef PRO
                   MessageBox (NULL, "\nWindows ZCS Device Switcher\n\n\nv1.0 pro, 25.01.2004 18:11\nby leshiy[cin!]\n\nLook at ReadMe.txt for extended info.\n\nwww.cyberpunk.net.ua\n" , "About WZCS", MB_OK);
               #else
                   MessageBox (NULL, "\nWindows ZCS Device Switcher\n\n\nv1.0 mini, 25.01.2004 18:11\nby leshiy[cin!]\n\nLook at ReadMe.txt for extended info.\n\nwww.cyberpunk.net.ua\n" , "About WZCS", MB_OK);
               #endif
               ShowState(hwnd,ST_OFF);                                   
            break;
            #ifdef PRO   
                case CM_ADD_378:
                     port=0x378;
                     ShowState(hwnd,pwr_on);                                   
                     SwitchPower();
                break;
                case CM_ADD_278:
                     port=0x278;
                     ShowState(hwnd,pwr_on);                                   
                     SwitchPower();
                break;
                case CM_ADD_3BC:
                     port=0x3BC;
                     ShowState(hwnd,pwr_on);                                   
                     SwitchPower();
                break;
                
                case CM_BIT+0x10:
                     for (int i=0;i<8; i++) bits_default[i]=1;
                     ShowState(hwnd,pwr_on);                                                        
                break;          
                case CM_BIT+0x11:
                     for (int i=0;i<8; i++) bits_default[i]=0;
                     ShowState(hwnd,pwr_on);                                   
                break;          

                case CM_DBIT+0x10:
                     for (int i=0;i<8; i++) bits_state[i]=!bits_state[i];
                     ShowState(hwnd,pwr_on);                                   
                     SwitchPower();
                break;          

                default: {
                    if ( (LOWORD(wParam)>=CM_BIT) && (LOWORD(wParam)<(CM_BIT+8))) 
                    {
                        int indx=LOWORD(wParam)-CM_BIT;
                        bits_default[indx]=!bits_default[indx];
                        ShowState(hwnd,pwr_on);
                    }
                    if ( (LOWORD(wParam)>=CM_DBIT) && (LOWORD(wParam)<(CM_DBIT+8))) 
                    {
                        int indx=LOWORD(wParam)-CM_DBIT;
                        bits_state[indx]=!bits_state[indx];
                        ShowState(hwnd,pwr_on);                                   
                        SwitchPower();
                    }
                    
                }
            #endif          
         }
        break;
       case WM_TRAY_CLICK:
             switch (lParam) {
                 case WM_LBUTTONDOWN:
                         SetPwrState(!pwr_on);
                         SwitchPower();
                         ShowState(hwnd,pwr_on);                                   
                        
                 break;
                 case WM_RBUTTONDOWN:
                     {
                            HMENU hmenu=GetMenu(hwnd);

                            HMENU hmenuTrackPopup;  // shortcut menu 
                            POINT pos;
                                                                                                                                                                                   
                            // Load the menu resource. 
   
                            // TrackPopupMenu cannot display the menu bar so get 
                            // a handle to the first shortcut menu. 
                            hmenuTrackPopup = GetSubMenu(hmenu, 0); 
                         
                            // Display the shortcut menu. Track the right mouse 
                            // button. 
                            GetCursorPos(&pos);
                            TrackPopupMenu(hmenuTrackPopup, 
                                    TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, 
                                    pos.x, pos.y, 0, hwnd, NULL); 
                            
                            // Destroy the menu. 
                         
                     }             
                 break;
             }
            break;    

        case WM_DESTROY:
             #ifdef PRO
                SaveParams();
             #endif 
             DeleteTrayIcon(hwnd);                                   
             PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
             UnLoadIOLib();
            break;
        default:                      /* for messages that we don't deal with */
             return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


