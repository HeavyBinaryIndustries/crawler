#define UNICODE

#include <windows.h>
#include <dirent.h>
#include <direct.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <stdio.h>
#include "myfunctions.h"
#include "add_word_to_dictionary.h"
#include "make_index_of_word.h"
#include "update_index.h"
#include "add_url.h"

LRESULT CALLBACK WndProc_main(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  static HWND list, content, edit_word, button_word, edit_url, button_url, button_clawling, button_update;
  DIR* dir;
  struct dirent *dp;
  static TCHAR text[256];
  static char** path_list;
  static int nlist;
  static char* multibyte_buf;
  static wchar_t* widechar_buf;
  static char path[256];
  int nread;
  FILE* fp;
  int i;
  char* base;
  int end;
  RECT rec;
  switch (uMsg) {
    case WM_DESTROY:
      for (i = 0; i < nlist; i++) {
        free(path_list[i]);
      }
      free(path_list);
      free(multibyte_buf);
      free(widechar_buf);
      PostQuitMessage(0);
      return 0;
    case WM_CREATE:
      multibyte_buf = malloc(0xffff);
      widechar_buf = malloc(sizeof(wchar_t) * 5120);
      setlocale(LC_ALL, ".UTF8");
      GetWindowRect(hwnd, &rec);
  		list = CreateWindow(TEXT("LISTBOX"), NULL,
                    			WS_CHILD | WS_VISIBLE | LBS_STANDARD,
                    			0, 0, 300, rec.bottom - rec.top - 70, hwnd, (HMENU)1,
                    			((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      dir = opendir("./index");
      if (dir != NULL) {
        path_list = malloc(sizeof(char*) * 4096);
        nlist = 0;
        while((dp = readdir(dir)) != NULL) {
          if (!strncmp(dp->d_name + strlen(dp->d_name) - 4, ".txt", 4)) {
            path_list[nlist] = malloc(strlen(dp->d_name) + 1);
            strcpy(path_list[nlist], dp->d_name);
            nlist++;
            strcpy(multibyte_buf, dp->d_name);
            hexagonal_to_string(multibyte_buf, multibyte_buf);
            MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
              multibyte_buf, -1, widechar_buf, MAX_WORD_LEN + 1);
            SendMessage(list, LB_ADDSTRING, 0, (LPARAM)widechar_buf);
          }
        }
        closedir(dir);
      }
      SendMessage(list, LB_SETCURSEL, 0, 0);
      content = CreateWindow(TEXT("LISTBOX"), NULL,
                              WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | WS_HSCROLL,
                              300, 0, rec.right - rec.left - 320, rec.bottom - rec.top - 100,
                              hwnd, (HMENU)2,
                              ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      edit_word = CreateWindow(TEXT("EDIT"), NULL,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
                                0, rec.bottom - rec.top - 105, 300, 30,
                                hwnd, (HMENU)3,
                                ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      button_word = CreateWindow(TEXT("BUTTON"), TEXT("検索語を追加"),
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                300, rec.bottom - rec.top - 105, 120, 30,
                                hwnd, (HMENU)4,
                                ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      edit_url = CreateWindow(TEXT("EDIT"), NULL,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
                                420, rec.bottom - rec.top - 105, rec.right - rec.left - 540, 30,
                                hwnd, (HMENU)5,
                                ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      button_url = CreateWindow(TEXT("BUTTON"), TEXT("URLを追加"),
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                rec.right - rec.left - 140, rec.bottom - rec.top - 105, 120, 30,
                                hwnd, (HMENU)6,
                                ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      button_clawling = CreateWindow(TEXT("BUTTON"), TEXT("クローリング開始"),
                                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      rec.right - rec.left - 220, rec.bottom - rec.top - 75, 200, 30,
                                      hwnd, (HMENU)7,
                                      ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      button_update = CreateWindow(TEXT("BUTTON"), TEXT("索引を更新"),
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    rec.right - rec.left - 420, rec.bottom - rec.top - 75, 200, 30,
                                    hwnd, (HMENU)8,
                                    ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
      return 0;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case 1:
          SendMessage(content, LB_RESETCONTENT, 0, 0);
          SendMessage(list, LB_GETTEXT, SendMessage(list, LB_GETCURSEL, 0, 0), (LPARAM)widechar_buf);
          WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, widechar_buf, -1, multibyte_buf, 0xffff, NULL, NULL);
          strcpy(path, "./index/");
          utf8_string_to_hexagonal_path(multibyte_buf, path + strlen(path));
          strcat(path, ".txt");
          fp = fopen(path, "r");
          if (fp != NULL) {
            nread = fread(multibyte_buf, 1, 0xfffe, fp);
            fclose(fp);
            multibyte_buf[nread] = '\0';
            base = multibyte_buf;
            while (1) {
              i = 0;
              while (base[i] != '\n' && base [i] != '\0') {
                i++;
              }
              end = base[i] == '\0';
              base[i] = '\0';
              MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
                base, i + 1, widechar_buf, 5120);
              SendMessage(content, LB_ADDSTRING, 0, (LPARAM)widechar_buf);
              if (end) break;
              base += i + 1;
            }
          }
          return 0;
        case 2:
          if (HIWORD(wParam) == LBN_DBLCLK) {
            SendMessage(content, LB_GETTEXT, SendMessage(content, LB_GETCURSEL, 0, 0), (LPARAM)widechar_buf);
            WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, widechar_buf, -1, multibyte_buf, 0xffff, NULL, NULL);
            i = 0;
            base = multibyte_buf;
            while (i < 4) {
              while (*base != '\0' && *base != ',') {
                base++;
              }
              base += *base == ',';
              i++;
            }
            base += *base == ' ';
            ShellExecuteA(NULL, "open", base, NULL, NULL, SW_SHOWNORMAL);
          }
          return 0;
        case 4:
          GetWindowText(edit_word, widechar_buf, MAX_WORD_LEN);
          if (!lstrlen(widechar_buf)) return 0;
          memset(multibyte_buf, 0, 0xffff);
          WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, widechar_buf, MAX_WORD_LEN, multibyte_buf, 0xffff, NULL, NULL);
          convert_utf8(multibyte_buf, multibyte_buf);
          MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, multibyte_buf, -1, widechar_buf, 0xffff);
          wsprintf(widechar_buf + MAX_WORD_LEN + 1, L"%lsを検索語に追加しますか？", widechar_buf);
          if (MessageBox(hwnd, widechar_buf + MAX_WORD_LEN + 1, TEXT("メッセージ"), MB_ICONINFORMATION | MB_YESNO) == IDYES) {
            int r = add_word_to_dictionary_utf8(multibyte_buf, 0);
            if (r < 0){
              MessageBox(hwnd, TEXT("文字列が不正です"), TEXT("メッセージ"), MB_ICONEXCLAMATION | MB_OK);
            } else if (r > 0) {
              wsprintf(widechar_buf + MAX_WORD_LEN + 1, L"%lsは既に登録されています", widechar_buf);
              MessageBox(hwnd, widechar_buf + MAX_WORD_LEN + 1, TEXT("メッセージ"), MB_ICONINFORMATION | MB_OK);
            } else {
              r = make_index_of_word(multibyte_buf, 100, 10, 1);
              if (r == 0) {
                wsprintf(widechar_buf + MAX_WORD_LEN + 1, L"%lsを検索語に追加し、索引を作成しました", widechar_buf);
                MessageBox(hwnd, widechar_buf + MAX_WORD_LEN + 1, TEXT("メッセージ"), MB_ICONINFORMATION | MB_OK);
              } else {
                MessageBox(hwnd, TEXT("索引の作成に失敗しました"), TEXT("メッセージ"), MB_ICONINFORMATION | MB_OK);
              }
            }
            SendMessage(list, LB_RESETCONTENT, 0, 0);
            SendMessage(content, LB_RESETCONTENT, 0, 0);
            dir = opendir("./index");
            if (dir != NULL) {
              path_list = malloc(sizeof(char*) * 4096);
              nlist = 0;
              while((dp = readdir(dir)) != NULL) {
                if (!strncmp(dp->d_name + strlen(dp->d_name) - 4, ".txt", 4)) {
                  path_list[nlist] = malloc(strlen(dp->d_name) + 1);
                  strcpy(path_list[nlist], dp->d_name);
                  nlist++;
                  strcpy(multibyte_buf, dp->d_name);
                  hexagonal_to_string(multibyte_buf, multibyte_buf);
                  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
                  multibyte_buf, -1, widechar_buf, MAX_WORD_LEN + 1);
                  SendMessage(list, LB_ADDSTRING, 0, (LPARAM)widechar_buf);
                }
              }
              closedir(dir);
            }
          }
          return 0;
        case 6:
          GetWindowText(edit_url, widechar_buf, URI_LEN);
          if (!lstrlen(widechar_buf)) return 0;
          wsprintf(widechar_buf + URI_LEN + 1, L"%lsをクローリング対象に追加しますか？", widechar_buf);
          if (MessageBox(hwnd, widechar_buf + URI_LEN + 1, TEXT("メッセージ"), MB_ICONINFORMATION | MB_YESNO) == IDYES) {
            WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, widechar_buf, URI_LEN, multibyte_buf, 0xffff, NULL, NULL);
            if (!add_url(multibyte_buf)) {
              wsprintf(widechar_buf + URI_LEN + 1, L"%lsをクローリング対象に追加しました", widechar_buf);
              MessageBox(hwnd, widechar_buf + URI_LEN + 1, TEXT("メッセージ"), MB_ICONINFORMATION | MB_OK);
            } else {
              MessageBox(hwnd, TEXT("URLの追加に失敗しました"), TEXT("メッセージ"), MB_ICONINFORMATION | MB_OK);
            }
          }
          return 0;
        case 7:
          ShellExecuteA(hwnd, "open", "crawl_in_domainlist.exe", NULL, NULL, SW_SHOWNORMAL);
          return 0;
        case 8:
          update_index(0xffff, 100, 10, 1);
          SendMessage(list, LB_RESETCONTENT, 0, 0);
          SendMessage(content, LB_RESETCONTENT, 0, 0);
          dir = opendir("./index");
          if (dir != NULL) {
            path_list = malloc(sizeof(char*) * 4096);
            nlist = 0;
            while((dp = readdir(dir)) != NULL) {
              if (!strncmp(dp->d_name + strlen(dp->d_name) - 4, ".txt", 4)) {
                path_list[nlist] = malloc(strlen(dp->d_name) + 1);
                strcpy(path_list[nlist], dp->d_name);
                nlist++;
                strcpy(multibyte_buf, dp->d_name);
                hexagonal_to_string(multibyte_buf, multibyte_buf);
                MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED,
                multibyte_buf, -1, widechar_buf, MAX_WORD_LEN + 1);
                SendMessage(list, LB_ADDSTRING, 0, (LPARAM)widechar_buf);
              }
            }
            closedir(dir);
          }
          return 0;
      }
      return 0;
    case WM_SIZE:
      GetWindowRect(hwnd, &rec);
      SetWindowPos(list, NULL, 0, 0, 300, rec.bottom - rec.top - 100, SWP_SHOWWINDOW);
      SetWindowPos(content, NULL, 300, 0, rec.right - rec.left - 320, rec.bottom - rec.top - 105, SWP_SHOWWINDOW);
      SetWindowPos(edit_word, NULL, 0, rec.bottom - rec.top - 105, 300, 30, SWP_SHOWWINDOW);
      SetWindowPos(button_word, NULL, 300, rec.bottom - rec.top - 105, 120, 30, SWP_SHOWWINDOW);
      SetWindowPos(edit_url, NULL, 420, rec.bottom - rec.top - 105, rec.right - rec.left - 560, 30, SWP_SHOWWINDOW);
      SetWindowPos(button_url, NULL, rec.right - rec.left - 140, rec.bottom - rec.top - 105, 120, 30, SWP_SHOWWINDOW);
      SetWindowPos(button_clawling, NULL, rec.right - rec.left - 220, rec.bottom - rec.top - 75, 200, 30, SWP_SHOWWINDOW);
      SetWindowPos(button_update, NULL, rec.right - rec.left - 420, rec.bottom - rec.top - 75, 200, 30, SWP_SHOWWINDOW);
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow) {
    mkdir("./main");
    chdir("./main");
    TCHAR* AppName = TEXT("Crawler_GUI");
    WNDCLASS wc_main;

    wc_main.style          = CS_HREDRAW | CS_VREDRAW;
    wc_main.lpfnWndProc    = WndProc_main;
    wc_main.cbClsExtra     = 0;
    wc_main.cbWndExtra     = 0;
    wc_main.hInstance      = hInstance;
    wc_main.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wc_main.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc_main.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
    wc_main.lpszMenuName   = NULL;
    wc_main.lpszClassName  = AppName;

    if (!RegisterClass(&wc_main)) return 0;


    HWND hwnd_main = CreateWindow(AppName, AppName,
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_THICKFRAME,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  NULL, NULL,
                  hInstance, NULL);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    chdir("../");
    return msg.wParam;
}
