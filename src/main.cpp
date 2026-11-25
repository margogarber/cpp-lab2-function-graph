#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <cmath>
#include <cwchar>
#include <limits>
#include <string>
#include <vector>

#define IDM_EXIT   1001
#define IDM_ABOUT  1002
#define IDM_FUNC1  1101
#define IDM_FUNC2  1102
#define IDM_FUNC3  1103

static const wchar_t kClassName[] = L"Lab2FunctionGraph";
static const wchar_t kTitle[]     = L"ЛР2 — Варіант 4 — Гарбєр М.О.";

static const int kWindowWidth  = 800;
static const int kWindowHeight = 600;

static const int kMarginLeft   = 70;
static const int kMarginRight  = 30;
static const int kMarginTop    = 60;
static const int kMarginBottom = 45;

static double FuncXSinX(double x)
{
    return x * std::sin(x);
}

static double FuncCubic(double x)
{
    return x * x * x - 6.0 * x * x + 4.0 * x + 12.0;
}

static double FuncDampedCos(double x)
{
    return std::exp(-x / 4.0) * std::cos(2.0 * x);
}

struct FunctionInfo
{
    double (*f)(double);
    double         xMin;
    double         xMax;
    const wchar_t* formula;
};

static const FunctionInfo kFunctions[] =
{
    { FuncXSinX,     -10.0, 10.0, L"f(x) = x · sin(x)" },
    { FuncCubic,      -2.0,  6.0, L"f(x) = x³ − 6x² + 4x + 12" },
    { FuncDampedCos,  -2.0, 10.0, L"f(x) = e^(−x/4) · cos(2x)" },
};

static const int kFunctionCount = sizeof(kFunctions) / sizeof(kFunctions[0]);

static HINSTANCE hInst;
static int       g_current = 0;

ATOM             MyRegisterClass(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    if (!MyRegisterClass(hInstance))
    {
        MessageBoxW(nullptr, L"Не вдалося зареєструвати клас вікна", kTitle, MB_ICONERROR);
        return FALSE;
    }
    if (!InitInstance(hInstance, nCmdShow))
    {
        MessageBoxW(nullptr, L"Не вдалося створити вікно", kTitle, MB_ICONERROR);
        return FALSE;
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize        = sizeof(WNDCLASSEXW);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wcex.hCursor       = LoadCursorW(nullptr, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcex.lpszClassName = kClassName;
    wcex.hIconSm       = LoadIconW(nullptr, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

static HMENU CreateMainMenu()
{
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    HMENU hFunc = CreatePopupMenu();
    HMENU hHelp = CreatePopupMenu();

    AppendMenuW(hFile, MF_STRING, IDM_EXIT, L"Вихід\tEsc");
    for (int i = 0; i < kFunctionCount; ++i)
    {
        std::wstring item = L"&" + std::to_wstring(i + 1) + L"  " + kFunctions[i].formula;
        AppendMenuW(hFunc, MF_STRING, IDM_FUNC1 + i, item.c_str());
    }
    AppendMenuW(hHelp, MF_STRING, IDM_ABOUT, L"Про програму");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, L"Файл");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFunc, L"Функція");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"Довідка");

    CheckMenuRadioItem(hFunc, IDM_FUNC1, IDM_FUNC1 + kFunctionCount - 1,
                       IDM_FUNC1 + g_current, MF_BYCOMMAND);
    return hMenu;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);

    HWND hWnd = CreateWindowW(kClassName, kTitle, WS_OVERLAPPEDWINDOW,
                              (cx - kWindowWidth) / 2, (cy - kWindowHeight) / 2,
                              kWindowWidth, kWindowHeight,
                              nullptr, CreateMainMenu(), hInstance, nullptr);
    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

static double NiceStep(double range, int maxTicks)
{
    if (maxTicks < 1)
        maxTicks = 1;
    double raw  = range / maxTicks;
    double mag  = std::pow(10.0, std::floor(std::log10(raw)));
    double norm = raw / mag;
    double nice;
    if (norm <= 1.0)      nice = 1.0;
    else if (norm <= 2.0) nice = 2.0;
    else if (norm <= 5.0) nice = 5.0;
    else                  nice = 10.0;
    return nice * mag;
}

static std::wstring FormatNumber(double value, double step)
{
    if (std::fabs(value) < step * 1e-6)
        value = 0.0;
    wchar_t buf[64];
    swprintf(buf, 64, L"%g", value);
    return buf;
}

static std::wstring FormatFixed(double value)
{
    wchar_t buf[64];
    swprintf(buf, 64, L"%.3f", value);
    return buf;
}

static void DrawGraph(HDC hdc, const RECT& rc)
{
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    const FunctionInfo& fn = kFunctions[g_current];

    int left   = rc.left + kMarginLeft;
    int right  = rc.right - kMarginRight;
    int top    = rc.top + kMarginTop;
    int bottom = rc.bottom - kMarginBottom;

    HFONT font    = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    if (right - left < 60 || bottom - top < 60)
    {
        SetTextAlign(hdc, TA_LEFT | TA_TOP);
        const wchar_t* msg = L"Вікно замале для побудови графіка";
        TextOutW(hdc, 10, 10, msg, (int)wcslen(msg));
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        return;
    }

    int samples = right - left + 1;
    std::vector<double> xs(samples), ys(samples);
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < samples; ++i)
    {
        double x = fn.xMin + (fn.xMax - fn.xMin) * i / (samples - 1);
        double y = fn.f(x);
        xs[i] = x;
        ys[i] = y;
        if (std::isfinite(y))
        {
            if (y < yMin) yMin = y;
            if (y > yMax) yMax = y;
        }
    }

    bool hasValues = yMin <= yMax;
    double dataMin = hasValues ? yMin : 0.0;
    double dataMax = hasValues ? yMax : 0.0;

    if (!hasValues)
    {
        yMin = -1.0;
        yMax = 1.0;
    }
    if (yMax - yMin < 1e-12)
    {
        yMin -= 1.0;
        yMax += 1.0;
    }
    double pad = (yMax - yMin) * 0.05;
    yMin -= pad;
    yMax += pad;

    auto toScreenX = [&](double x) {
        return left + (int)std::lround((x - fn.xMin) / (fn.xMax - fn.xMin) * (right - left));
    };
    auto toScreenY = [&](double y) {
        return bottom - (int)std::lround((y - yMin) / (yMax - yMin) * (bottom - top));
    };

    HPEN gridPen  = CreatePen(PS_DOT, 1, RGB(190, 190, 190));
    HPEN framePen = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
    HPEN axisPen  = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN graphPen = CreatePen(PS_SOLID, 2, RGB(0, 90, 200));
    HPEN oldPen   = (HPEN)SelectObject(hdc, gridPen);

    double stepX = NiceStep(fn.xMax - fn.xMin, (right - left) / 80);
    double stepY = NiceStep(yMax - yMin, (bottom - top) / 50);

    SetTextAlign(hdc, TA_CENTER | TA_TOP);
    int kxFirst = (int)std::ceil(fn.xMin / stepX - 1e-9);
    int kxLast  = (int)std::floor(fn.xMax / stepX + 1e-9);
    for (int k = kxFirst; k <= kxLast; ++k)
    {
        double x  = (double)k * stepX;
        int    px = toScreenX(x);
        MoveToEx(hdc, px, top, nullptr);
        LineTo(hdc, px, bottom);
        std::wstring label = FormatNumber(x, stepX);
        TextOutW(hdc, px, bottom + 6, label.c_str(), (int)label.size());
    }

    SetTextAlign(hdc, TA_RIGHT | TA_TOP);
    int kyFirst = (int)std::ceil(yMin / stepY - 1e-9);
    int kyLast  = (int)std::floor(yMax / stepY + 1e-9);
    for (int k = kyFirst; k <= kyLast; ++k)
    {
        double y  = (double)k * stepY;
        int    py = toScreenY(y);
        MoveToEx(hdc, left, py, nullptr);
        LineTo(hdc, right, py);
        std::wstring label = FormatNumber(y, stepY);
        TextOutW(hdc, left - 8, py - 8, label.c_str(), (int)label.size());
    }

    SelectObject(hdc, framePen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, left, top, right + 1, bottom + 1);
    SelectObject(hdc, oldBrush);

    SelectObject(hdc, axisPen);
    SetTextAlign(hdc, TA_LEFT | TA_TOP);
    if (fn.xMin <= 0.0 && fn.xMax >= 0.0)
    {
        int px = toScreenX(0.0);
        MoveToEx(hdc, px, bottom, nullptr);
        LineTo(hdc, px, top);
        TextOutW(hdc, px + 6, top + 2, L"y", 1);
    }
    if (yMin <= 0.0 && yMax >= 0.0)
    {
        int py = toScreenY(0.0);
        MoveToEx(hdc, left, py, nullptr);
        LineTo(hdc, right, py);
        TextOutW(hdc, right - 14, py - 20, L"x", 1);
    }

    HRGN clip = CreateRectRgn(left, top, right + 1, bottom + 1);
    SelectClipRgn(hdc, clip);
    SelectObject(hdc, graphPen);

    std::vector<POINT> segment;
    segment.reserve(samples);
    for (int i = 0; i <= samples; ++i)
    {
        bool valid = i < samples && std::isfinite(ys[i]);
        if (valid)
        {
            POINT p = { toScreenX(xs[i]), toScreenY(ys[i]) };
            segment.push_back(p);
        }
        else
        {
            if (segment.size() >= 2)
                Polyline(hdc, segment.data(), (int)segment.size());
            segment.clear();
        }
    }

    SelectClipRgn(hdc, nullptr);
    DeleteObject(clip);

    SetTextAlign(hdc, TA_LEFT | TA_TOP);
    SetTextColor(hdc, RGB(0, 90, 200));
    std::wstring line1 = std::wstring(fn.formula);
    TextOutW(hdc, left, 8, line1.c_str(), (int)line1.size());

    SetTextColor(hdc, RGB(60, 60, 60));
    std::wstring line2 = L"x ∈ [" + FormatFixed(fn.xMin) + L"; " + FormatFixed(fn.xMax) +
                         L"]    y ∈ [" + FormatFixed(dataMin) + L"; " + FormatFixed(dataMax) +
                         L"]    крок сітки: Δx = " + FormatNumber(stepX, stepX) +
                         L", Δy = " + FormatNumber(stepY, stepY);
    TextOutW(hdc, left, 30, line2.c_str(), (int)line2.size());

    SelectObject(hdc, oldPen);
    DeleteObject(gridPen);
    DeleteObject(framePen);
    DeleteObject(axisPen);
    DeleteObject(graphPen);

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

static void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT rc;
    GetClientRect(hWnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    if (w > 0 && h > 0)
    {
        HDC     memDC  = CreateCompatibleDC(hdc);
        HBITMAP bmp    = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, bmp);

        DrawGraph(memDC, rc);
        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(bmp);
        DeleteDC(memDC);
    }

    EndPaint(hWnd, &ps);
}

static void SelectFunction(HWND hWnd, int index)
{
    g_current = index;
    HMENU hFunc = GetSubMenu(GetMenu(hWnd), 1);
    CheckMenuRadioItem(hFunc, IDM_FUNC1, IDM_FUNC1 + kFunctionCount - 1,
                       IDM_FUNC1 + index, MF_BYCOMMAND);
    InvalidateRect(hWnd, nullptr, FALSE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        if (id >= IDM_FUNC1 && id < IDM_FUNC1 + kFunctionCount)
        {
            SelectFunction(hWnd, id - IDM_FUNC1);
            break;
        }
        switch (id)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_ABOUT:
            MessageBoxW(hWnd,
                        L"Лабораторна робота №2, варіант 4\n"
                        L"Побудова графіка функції з автоматичним масштабуванням\n"
                        L"Гарбєр Маргарита Олександрівна, ІА-з41",
                        L"Про програму", MB_OK | MB_ICONINFORMATION);
            break;
        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
            DestroyWindow(hWnd);
        else if ((int)wParam >= '1' && (int)wParam < '1' + kFunctionCount)
            SelectFunction(hWnd, (int)wParam - '1');
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 360;
        mmi->ptMinTrackSize.y = 300;
        break;
    }

    case WM_SIZE:
        InvalidateRect(hWnd, nullptr, FALSE);
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        OnPaint(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
