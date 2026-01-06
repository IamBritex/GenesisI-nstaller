#pragma once
#include <windows.h>
#include <math.h>

#define PI 3.14159265

class LoadingScreen {
public:
    // Estado de la animación (ángulo de rotación)
    static int rotationAngle;

    static void Draw(HWND hWnd, HDC hdc) {
        RECT rc; GetClientRect(hWnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        // Fondo Negro
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);

        // Configuración del Spinner
        int centerX = width / 2;
        int centerY = height / 2;
        int radius = 30;
        int thickness = 4;

        // Usar GDI para dibujar un arco giratorio
        HPEN hPenBase = CreatePen(PS_SOLID, thickness, RGB(50, 50, 50)); // Gris oscuro (fondo del track)
        HPEN hPenActive = CreatePen(PS_SOLID, thickness, RGB(0, 255, 136)); // Verde Genesis (activo)
        
        HGDIOBJ oldPen = SelectObject(hdc, hPenBase);
        SelectObject(hdc, GetStockObject(NULL_BRUSH)); // Sin relleno

        // Dibujar circulo base
        Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);

        // Dibujar arco giratorio (Simulado con lineas para simplificar GDI puro)
        SelectObject(hdc, hPenActive);
        
        // Dibujamos un arco de 90 grados que rota
        for (int i = 0; i < 90; i+=5) {
            int angle = (rotationAngle + i) % 360;
            double rad = angle * PI / 180.0;
            int x = centerX + (int)(cos(rad) * radius);
            int y = centerY + (int)(sin(rad) * radius);
            
            if (i == 0) MoveToEx(hdc, x, y, NULL);
            else LineTo(hdc, x, y);
        }

        // Texto "LOADING"
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(150, 150, 150));
        const wchar_t* text = L"CARGANDO...";
        SIZE textSize; GetTextExtentPoint32W(hdc, text, wcslen(text), &textSize);
        TextOutW(hdc, centerX - (textSize.cx / 2), centerY + radius + 15, text, wcslen(text));

        SelectObject(hdc, oldPen);
        DeleteObject(hPenBase);
        DeleteObject(hPenActive);
    }
};

int LoadingScreen::rotationAngle = 0;