#define UNICODE
#define _UNICODE
#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <string>

// Структура для игры в библиотеке
struct GameItem {
    std::wstring title;
    std::wstring playTime;
    std::wstring lastPlayed;
    std::wstring status;
};

static std::vector<GameItem> g_games = {
    { L"Counter-Strike 2", L"1,420 ч.", L"Сегодня", L"Готово к игре" },
    { L"Dota 2", L"890 ч.", L"Вчера", L"Требуется обновление" },
    { L"Half-Life 3 (Beta)", L"12 ч.", L"30 сен 2026", L"Установлено" },
    { L"Cyberpunk 2077", L"95 ч.", L"15 авг 2026", L"Готово к игре" },
    { L"Team Fortress 2", L"340 ч.", L"12 мар 2025", L"Готово к игре" },
    { L"Grand Theft Auto V", L"210 ч.", L"1 янв 2026", L"Готово к игре" }
};

static int g_selectedGame = 0;
static RECT g_rcPlayBtn = { 0 };
static bool g_isHoverPlay = false;

// Цветовая палитра стиля Steam
const COLORREF CLR_HEADER_BG    = RGB(23, 26, 33);
const COLORREF CLR_SIDEBAR_BG   = RGB(18, 20, 24);
const COLORREF CLR_MAIN_BG      = RGB(27, 40, 56);
const COLORREF CLR_CARD_BG      = RGB(33, 44, 61);
const COLORREF CLR_ACCENT_BLUE  = RGB(102, 192, 244);
const COLORREF CLR_TEXT_WHITE   = RGB(235, 235, 235);
const COLORREF CLR_TEXT_MUTED   = RGB(143, 152, 160);
const COLORREF CLR_GREEN_BTN    = RGB(92, 126, 16);
const COLORREF CLR_GREEN_HOVER  = RGB(117, 156, 22);
const COLORREF CLR_ITEM_ACTIVE  = RGB(42, 71, 94);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT clientRc;
        GetClientRect(hwnd, &clientRc);
        int width = clientRc.right;
        int height = clientRc.bottom;

        // Двойная буферизация против мерцания экрана
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

        HFONT hFontLogo = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hFontMenu = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hFontMain = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hFontTitle = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hFontPlay = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

        SetBkMode(memDC, TRANSPARENT);

        // 1. Верхний бар (Header)
        RECT rcHeader = { 0, 0, width, 55 };
        HBRUSH brHeader = CreateSolidBrush(CLR_HEADER_BG);
        FillRect(memDC, &rcHeader, brHeader);
        DeleteObject(brHeader);

        SelectObject(memDC, hFontLogo);
        SetTextColor(memDC, CLR_TEXT_WHITE);
        TextOutW(memDC, 20, 15, L"PinSte", 6);

        SelectObject(memDC, hFontMenu);
        SetTextColor(memDC, CLR_ACCENT_BLUE);
        TextOutW(memDC, 120, 18, L"МАГАЗИН", 7);
        SetTextColor(memDC, CLR_TEXT_WHITE);
        TextOutW(memDC, 220, 18, L"БИБЛИОТЕКА", 10);
        SetTextColor(memDC, CLR_TEXT_MUTED);
        TextOutW(memDC, 350, 18, L"СООБЩЕСТВО", 10);
        TextOutW(memDC, 480, 18, L"ПРОФИЛЬ", 7);

        SetTextColor(memDC, CLR_ACCENT_BLUE);
        TextOutW(memDC, width - 150, 18, L"User (В сети)", 13);

        // 2. Левая колонка (Список игр)
        int sidebarWidth = 260;
        RECT rcSidebar = { 0, 55, sidebarWidth, height - 35 };
        HBRUSH brSidebar = CreateSolidBrush(CLR_SIDEBAR_BG);
        FillRect(memDC, &rcSidebar, brSidebar);
        DeleteObject(brSidebar);

        SelectObject(memDC, hFontMain);
        SetTextColor(memDC, CLR_TEXT_MUTED);
        TextOutW(memDC, 15, 68, L"КОЛЛЕКЦИИ (6)", 13);

        int itemY = 95;
        int itemH = 34;
        for (size_t i = 0; i < g_games.size(); ++i) {
            RECT itemRc = { 10, itemY, sidebarWidth - 10, itemY + itemH };
            if ((int)i == g_selectedGame) {
                HBRUSH brActive = CreateSolidBrush(CLR_ITEM_ACTIVE);
                FillRect(memDC, &itemRc, brActive);
                DeleteObject(brActive);
                SetTextColor(memDC, CLR_TEXT_WHITE);
            } else {
                SetTextColor(memDC, CLR_TEXT_MUTED);
            }

            RECT textRc = { itemRc.left + 10, itemRc.top + 7, itemRc.right, itemRc.bottom };
            DrawTextW(memDC, g_games[i].title.c_str(), -1, &textRc, DT_LEFT | DT_SINGLELINE);
            itemY += itemH + 2;
        }

        // 3. Основная область деталей
        RECT rcMain = { sidebarWidth, 55, width, height - 35 };
        HBRUSH brMain = CreateSolidBrush(CLR_MAIN_BG);
        FillRect(memDC, &rcMain, brMain);
        DeleteObject(brMain);

        const auto& curGame = g_games[g_selectedGame];

        SelectObject(memDC, hFontTitle);
        SetTextColor(memDC, CLR_TEXT_WHITE);
        TextOutW(memDC, sidebarWidth + 30, 80, curGame.title.c_str(), (int)curGame.title.length());

        // Кнопка "ИГРАТЬ"
        g_rcPlayBtn.left = sidebarWidth + 30;
        g_rcPlayBtn.top = 135;
        g_rcPlayBtn.right = sidebarWidth + 180;
        g_rcPlayBtn.bottom = 180;

        HBRUSH brPlay = CreateSolidBrush(g_isHoverPlay ? CLR_GREEN_HOVER : CLR_GREEN_BTN);
        FillRect(memDC, &g_rcPlayBtn, brPlay);
        DeleteObject(brPlay);

        SelectObject(memDC, hFontPlay);
        SetTextColor(memDC, CLR_TEXT_WHITE);
        RECT rcPlayText = g_rcPlayBtn;
        DrawTextW(memDC, L"ИГРАТЬ", -1, &rcPlayText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Карточка статистики
        SelectObject(memDC, hFontMain);
        RECT rcCard = { sidebarWidth + 30, 210, width - 30, 310 };
        HBRUSH brCard = CreateSolidBrush(CLR_CARD_BG);
        FillRect(memDC, &rcCard, brCard);
        DeleteObject(brCard);

        SetTextColor(memDC, CLR_TEXT_MUTED);
        TextOutW(memDC, rcCard.left + 20, rcCard.top + 20, L"ПОСЛЕДНИЙ ЗАПУСК", 16);
        TextOutW(memDC, rcCard.left + 220, rcCard.top + 20, L"ВРЕМЯ В ИГРЕ", 12);
        TextOutW(memDC, rcCard.left + 420, rcCard.top + 20, L"СОСТОЯНИЕ", 9);

        SetTextColor(memDC, CLR_TEXT_WHITE);
        TextOutW(memDC, rcCard.left + 20, rcCard.top + 45, curGame.lastPlayed.c_str(), (int)curGame.lastPlayed.length());
        TextOutW(memDC, rcCard.left + 220, rcCard.top + 45, curGame.playTime.c_str(), (int)curGame.playTime.length());
        SetTextColor(memDC, CLR_ACCENT_BLUE);
        TextOutW(memDC, rcCard.left + 420, rcCard.top + 45, curGame.status.c_str(), (int)curGame.status.length());

        // 4. Подвал (Footer)
        RECT rcFooter = { 0, height - 35, width, height };
        HBRUSH brFooter = CreateSolidBrush(CLR_HEADER_BG);
        FillRect(memDC, &rcFooter, brFooter);
        DeleteObject(brFooter);

        SelectObject(memDC, hFontMain);
        SetTextColor(memDC, CLR_TEXT_MUTED);
        TextOutW(memDC, 15, height - 26, L"+ ДОБАВИТЬ ИГРУ", 15);
        TextOutW(memDC, width / 2 - 40, height - 26, L"ЗАГРУЗКИ (0)", 12);
        TextOutW(memDC, width - 140, height - 26, L"ДРУЗЬЯ И ЧАТ", 12);

        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        DeleteObject(hFontLogo);
        DeleteObject(hFontMenu);
        DeleteObject(hFontMain);
        DeleteObject(hFontTitle);
        DeleteObject(hFontPlay);
        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        bool hover = (x >= g_rcPlayBtn.left && x <= g_rcPlayBtn.right &&
                      y >= g_rcPlayBtn.top  && y <= g_rcPlayBtn.bottom);
        if (hover != g_isHoverPlay) {
            g_isHoverPlay = hover;
            InvalidateRect(hwnd, &g_rcPlayBtn, FALSE);
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (x >= g_rcPlayBtn.left && x <= g_rcPlayBtn.right &&
            y >= g_rcPlayBtn.top  && y <= g_rcPlayBtn.bottom) {
            std::wstring msgText = L"Запуск игры «" + g_games[g_selectedGame].title + L"» через клиент PinSte...";
            MessageBoxW(hwnd, msgText.c_str(), L"PinSte Launcher", MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        if (x >= 10 && x <= 250 && y >= 95) {
            int clickedIdx = (y - 95) / 36;
            if (clickedIdx >= 0 && clickedIdx < (int)g_games.size()) {
                g_selectedGame = clickedIdx;
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"PinSteMainWndClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"PinSte",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1020, 660,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
