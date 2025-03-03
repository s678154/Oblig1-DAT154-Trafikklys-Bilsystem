// Oblig1-1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Oblig1-1.h"
#include "resource.h"
#include <cstdlib>  // For rand(), srand()
#include <vector>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


enum CycleState { STATE0, STATE1, STATE2, STATE3 };
CycleState cycleState = STATE0;
CycleState currentTrafficState = STATE0;  // Brukes i biloppdateringen

// Representerer en bil
struct Bil {
    int x, y;         // Posisjon øverste venstre hjørne
    int w, h;         // Størrelse
    int dx, dy;       // hvor mye bilen flytter seg (ift timer) 
    COLORREF color;   // Farge
};

std::vector<Bil> biler;  // Liste av biler

double pw = 0.5; // Standard: 50% sjanse per sekund for biler fra vest
double pn = 0.5; // Standard: 50% sjanse per sekund for biler fra nord


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ProbDlgProc(HWND, UINT, WPARAM, LPARAM);

// Returnerer true dersom et rektangel (definert med left, top, right, bottom)
// overlapper med posisjonen for en av bilene i 'biler'-vektoren.
bool IsAreaOccupied(int left, int top, int right, int bottom, const std::vector<Bil>& biler)
{
    for (const auto& b : biler)
    {
        int bLeft = b.x;
        int bRight = b.x + b.w;
        int bTop = b.y;
        int bBottom = b.y + b.h;
        if (left < bRight && right > bLeft && top < bBottom && bottom > bTop)
            return true;
    }
    return false;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OBLIG11, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OBLIG11));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OBLIG11));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_OBLIG11);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_SET_PROB:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_PROB_DIALOG), hWnd, ProbDlgProc);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CREATE:
        // Timer 1: Oppdaterer trafikklysene hvert 1500 ms.
        SetTimer(hWnd, 1, 1500, NULL);
        // Timer 2: Oppdaterer bilbevegelsen hvert 50 ms for jevn animasjon.
        SetTimer(hWnd, 2, 50, NULL);
        // Timer 3: Legger til nye biler hvert 1000 ms.
        SetTimer(hWnd, 3, 1000, NULL);
        break;
    case WM_PAINT:
        {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect); // Henter vindusstørrelse

        int midX = rect.right / 2;  // Midten av skjermen i X-retning
        int midY = rect.bottom / 2; // Midten av skjermen i Y-retning
        int roadWidth = 100;        // Bredden på veien
        int roadHeight = 100;       // Høyden på veien
        int boxWidth = 30;          // Trafikklysboksene sin bredde
        int boxHeight = 120;        // Trafikklysboksebe sin hoyde

        // Tegne veiene
        HBRUSH roadBrush = CreateSolidBrush(RGB(150, 150, 150));
        RECT horisontalRoad = { 0, midY - roadHeight / 2, rect.right, midY + roadHeight / 2 };
        FillRect(hdc, &horisontalRoad, roadBrush); //horisontal vei
        RECT vertikalRoad = { midX - roadWidth / 2, 0, midX + roadWidth / 2, rect.bottom };
        FillRect(hdc, &vertikalRoad, roadBrush); //vertikal vei
        DeleteObject(roadBrush);

        // Tegne trafikklys-bokser 
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        SelectObject(hdc, blackBrush);
        int shift = 65;
        Rectangle(hdc, midX - boxWidth / 2 - shift, midY - roadHeight / 2 - boxHeight, midX + boxWidth / 2 - shift, midY - roadHeight / 2);
        Rectangle(hdc, midX - roadWidth / 2 - boxHeight, midY - boxWidth / 2 + shift, midX - roadWidth / 2, midY + boxWidth / 2 + shift);
        DeleteObject(blackBrush);

        // Lage brushes:
        HBRUSH horGreenBrush = CreateSolidBrush((cycleState == STATE0) ? RGB(0, 255, 0) : RGB(100, 100, 100));
        HBRUSH horYellowBrush = CreateSolidBrush(((cycleState == STATE1) || (cycleState == STATE3)) ? RGB(255, 255, 0) : RGB(100, 100, 100));
        HBRUSH horRedBrush = CreateSolidBrush(((cycleState == STATE2) || (cycleState == STATE3)) ? RGB(255, 0, 0) : RGB(100, 100, 100));
        // Bestem fargene for vertikale lys basert på cycleState:
        HBRUSH verRedBrush = CreateSolidBrush(((cycleState == STATE0) || (cycleState == STATE1)) ? RGB(255, 0, 0) : RGB(100, 100, 100));
        HBRUSH verYellowBrush = CreateSolidBrush(((cycleState == STATE1) || (cycleState == STATE3)) ? RGB(255, 255, 0) : RGB(100, 100, 100));
        HBRUSH verGreenBrush = CreateSolidBrush((cycleState == STATE2) ? RGB(0, 255, 0) : RGB(100, 100, 100));

        //Tegne lys-sirklene
        SelectObject(hdc, horRedBrush);
        Ellipse(hdc, midX + roadWidth / 2 - 125, midY - 155, midX + roadWidth / 2 - 105, midY - 135);
        SelectObject(hdc, horYellowBrush);
        Ellipse(hdc, midX + roadWidth / 2 - 125, midY - 115, midX + roadWidth / 2 - 105, midY - 95);
        SelectObject(hdc, horGreenBrush);
        Ellipse(hdc, midX + roadWidth / 2 - 125, midY - 75, midX + roadWidth / 2 - 105, midY - 55);
        // Vertikale lys (fra venstre til høyre: nå: GRØNN, GUL, RØD)
        SelectObject(hdc, verGreenBrush);
        Ellipse(hdc, midX - 155, midY + roadHeight / 2 + 5, midX - 135, midY + roadHeight / 2 + 25);
        SelectObject(hdc, verYellowBrush);
        Ellipse(hdc, midX - 115, midY + roadHeight / 2 + 5, midX - 95, midY + roadHeight / 2 + 25);
        SelectObject(hdc, verRedBrush);
        Ellipse(hdc, midX - 75, midY + roadHeight / 2 + 5, midX - 55, midY + roadHeight / 2 + 25);

        // 4) Tegne biler  
        for (auto& b : biler)
        {
            HBRUSH bilBrush = CreateSolidBrush(b.color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bilBrush);

            // Rektangel for bil
            Rectangle(hdc, b.x, b.y, b.x + b.w, b.y + b.h);

            SelectObject(hdc, oldBrush);
            DeleteObject(bilBrush);
        }

        // Rydde opp penslene:
        DeleteObject(horGreenBrush);
        DeleteObject(horYellowBrush);
        DeleteObject(horRedBrush);
        DeleteObject(verRedBrush);
        DeleteObject(verYellowBrush);
        DeleteObject(verGreenBrush);

        EndPaint(hWnd, &ps);
    }
        break;
    case WM_TIMER:
    {
        if (wParam == 1) { // Oppdaterer trafikklysene
            if (cycleState == STATE0)
                cycleState = STATE1;
            else if (cycleState == STATE1)
                cycleState = STATE2;
            else if (cycleState == STATE2)
                cycleState = STATE3;
            else
                cycleState = STATE0;

            currentTrafficState = cycleState;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 2) { // Oppdater bilbevegelsen
            RECT rect;
            GetClientRect(hWnd, &rect);
            int midX = rect.right / 2;
            int midY = rect.bottom / 2;
            int roadWidth = 100;
            int roadHeight = 100;
            // Definer stoppområdene før trafikklysene (midten) 
            int stopZoneHorizontal = midX - roadWidth / 2 - 20;
            int stopZoneVertical = midY - roadHeight / 2 - 20;

            double dt = 0.05; // Tidssteg (bruker 0.05 pga 50 ms)

            // Oppdater posisjon for hver bil
            for (auto& b : biler) {
                bool mustStop = false; // for rødt lys
                // For horisontale biler (fra venstre, dx > 0)
                if (b.dx > 0) {
                    // Sjekk om bilen nærmer seg krysset
                    if ((b.x + b.w) >= stopZoneHorizontal && (b.x + b.w) < midX - roadWidth / 2 + 5) {
                        // Hvis det horisontale lyset IKKE er grønt (STATE2 er grønt), stopp bilen
                        if (currentTrafficState != STATE2) {
                            mustStop = true;
                        }
                    }
                }
                // For vertikale biler (fra toppen, dy > 0)
                else if (b.dy > 0) {
                    if ((b.y + b.h) >= stopZoneVertical && (b.y + b.h) < midY - roadHeight / 2 + 5) {
                        // Hvis det vertikale lyset IKKE er grønt (STATE0 er grønt), stopp bilen
                        if (currentTrafficState != STATE0) {
                            mustStop = true;
                        }
                    }
                }
                // Hvis rødt lys -> hopp over
                if (mustStop) continue;

                // Regne ut neste posisjon
                double nextX = b.x + b.dx * dt;
                double nextY = b.y + b.dy * dt;

                // Sjekker om det vil oppstå en kollisjon
                bool collision = false;
                // Posisjon for denne bilens neste posisjon
                int left = (int)nextX;
                int right = (int)(nextX + b.w);
                int top = (int)nextY;
                int bottom = (int)(nextY + b.h);

                for (auto& other : biler) {
                    if (&other == &b) continue; // Ikke sammenlign med seg selv

                    // Andre bilens nåværende posisjon
                    int leftO = other.x;
                    int rightO = other.x + other.w;
                    int topO = other.y;
                    int bottomO = other.y + other.h;

                    const int gap = 10;
                    // Sjekker om de overlapper 
                    bool overlap = (left < rightO + gap && right > leftO - gap &&
                        top < bottomO + gap && bottom > topO - gap);
                    if (overlap) {
                        collision = true;
                        break;
                    }
                }

                // Hvis ikke kollisjon, flytt bilen
                if (!collision) {
                    b.x = nextX;
                    b.y = nextY;
                }
                // Hvis collision == true, gjør vi ingenting
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 3) { // Automatisert bilankomst hvert sekund
            RECT rect;
            GetClientRect(hWnd, &rect);
            int midX = rect.right / 2;
            int midY = rect.bottom / 2;

            // Dette er for å sjekke om nye biler skal bli lagt til eller ikke (om det er mye kø må de ikke bli lagt til)
            int spawnWestLeft = 0;
            int spawnWestTop = midY + 10;
            int spawnWestRight = 40;
            int spawnWestBottom = midY + 30;

            int spawnNorthLeft = midX - 10;
            int spawnNorthTop = 0;
            int spawnNorthRight = midX + 10;
            int spawnNorthBottom = 40;

            // Sjekk og spawn bil fra vest dersom området er ledig
            if (!IsAreaOccupied(spawnWestLeft, spawnWestTop, spawnWestRight, spawnWestBottom, biler) &&
                (((double)rand() / RAND_MAX) < pw))
            {
                Bil b;
                b.w = 30; b.h = 20;
                b.x = spawnWestLeft; // eller 0
                b.y = spawnWestTop;
                b.dx = 100; b.dy = 0;
                b.color = RGB(rand() % 256, rand() % 256, rand() % 256);
                biler.push_back(b);
            }

            // Sjekk og spawn bil fra nord dersom området er ledig
            if (!IsAreaOccupied(spawnNorthLeft, spawnNorthTop, spawnNorthRight, spawnNorthBottom, biler) &&
                (((double)rand() / RAND_MAX) < pn))
            {
                Bil b;
                b.w = 20; b.h = 30;
                b.x = spawnNorthLeft;
                b.y = spawnNorthTop;
                b.dx = 0; b.dy = 100;
                b.color = RGB(rand() % 256, rand() % 256, rand() % 256);
                biler.push_back(b);
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }
        }
        break;
    case WM_KEYDOWN:
        {
        switch (wParam)
        {
        case VK_LEFT:
            // Reduser pw med 10%
            pw -= 0.1;
            if (pw < 0.0) pw = 0.0;
            break;
        case VK_RIGHT:
            // Øk pw med 10%
            pw += 0.1;
            if (pw > 1.0) pw = 1.0;
            break;
        case VK_UP:
            // Øk pn med 10%
            pn += 0.1;
            if (pn > 1.0) pn = 1.0;
            break;
        case VK_DOWN:
            // Reduser pn med 10%
            pn -= 0.1;
            if (pn < 0.0) pn = 0.0;
            break;
        }
        InvalidateRect(hWnd, NULL, TRUE); // Tegn på nytt for å vise oppdaterte verdier (evt. hvis du tegner dem)
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK ProbDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        wchar_t buf[32];
        swprintf_s(buf, L"%.2f", pw);
        SetDlgItemText(hDlg, IDC_EDIT_PW, buf);
        swprintf_s(buf, L"%.2f", pn);
        SetDlgItemText(hDlg, IDC_EDIT_PN, buf);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            // Les ut verdier fra Edit-kontroller:
            wchar_t buf[32];
            GetDlgItemText(hDlg, IDC_EDIT_PW, buf, 32);
            double newPw = _wtof(buf);
            GetDlgItemText(hDlg, IDC_EDIT_PN, buf, 32);
            double newPn = _wtof(buf);

            // Sett globale variabler
            if (newPw >= 0.0 && newPw <= 1.0 && newPn >= 0.0 && newPn <= 1.0)
            {
                pw = newPw;
                pn = newPn;
                EndDialog(hDlg, IDOK);
            }
            else
            {
                MessageBox(hDlg, L"Må være mellom 0.0 og 1.0", L"Error", MB_OK);
            }
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
