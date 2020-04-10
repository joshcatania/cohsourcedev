#ifndef _EMBEDBROWSER_H
#define _EMBEDBROWSER_H

void UnEmbedBrowserObject(HWND hwnd);
void DoPageAction(HWND hwnd, DWORD action);
long DisplayHTMLStr(HWND hwnd, LPCTSTR string);
long DisplayHTMLPage(HWND hwnd, LPTSTR webPageName);
void ResizeBrowser(HWND hwnd, DWORD width, DWORD height);
long EmbedBrowserObject(HWND hwnd);

#endif
