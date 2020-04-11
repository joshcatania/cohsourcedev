#ifndef _OSDEPENDENT_H
#define _OSDEPENDENT_H


extern int g_isUsingWin2korXP;

void InitOSInfo(void);


// Call the correct SetDlgItemText function based on the user's OS
#define SET_DLG_ITEM_TEXT( hDlg, dlgItemID, str ) \
	g_isUsingWin2korXP ? SetDlgItemTextW(hDlg, dlgItemID, xlateToUnicode(str)) : SetDlgItemTextA(hDlg, dlgItemID, str) 

// Call the correct SetWindowText function based on the user's OS
#define SET_WINDOW_TEXT( hWnd, str ) \
	g_isUsingWin2korXP ? SetWindowTextW(hWnd, xlateToUnicode(str)) : SetWindowTextA(hWnd, str) 

// Call the correct GetWindowText function based on the user's OS
// pass in a wide character buffer 
#define GET_WINDOW_TEXT( hWnd, wc_buf, buf_size ) \
	g_isUsingWin2korXP ? GetWindowTextW(hWnd, wc_buf, buf_size) : GetWindowText(hWnd, (char*)wc_buf, buf_size >> 1)

// Call the correct DrawText function based on the user's OS
// pass in a wide character buffer 
#define DRAW_TEXT( hDC, wc_str, rect, format ) \
	g_isUsingWin2korXP ? DrawTextW(hDC, wc_str, wcslen(wc_str), rect, format) : \
	DrawText(hDC, (char*)wc_str, strlen((char*)wc_str), rect, format)



#endif