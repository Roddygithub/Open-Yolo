#pragma once

// Désactiver les avertissements pour X11
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Définir X_DISPLAY_MISSING avant d'inclure Xlib.h
#define X_DISPLAY_MISSING

// Inclure les en-têtes X11
#include <X11/Xlib.h>
#include <X11/keysym.h>

// Réactiver les avertissements
#pragma GCC diagnostic pop

namespace input {

// Définition des codes de touches
namespace Key {
    // Touches de contrôle
    constexpr unsigned int Ctrl = XK_Control_L;
    constexpr unsigned int Alt = XK_Alt_L;
    constexpr unsigned int Shift = XK_Shift_L;
    constexpr unsigned int Super = XK_Super_L;
    
    // Touches de direction
    constexpr unsigned int Up = XK_Up;
    constexpr unsigned int Down = XK_Down;
    constexpr unsigned int Left = XK_Left;
    constexpr unsigned int Right = XK_Right;
    
    // Touches spéciales
    constexpr unsigned int Space = XK_space;
    constexpr unsigned int Enter = XK_Return;
    constexpr unsigned int Escape = XK_Escape;
    constexpr unsigned int Tab = XK_Tab;
    constexpr unsigned int Backspace = XK_BackSpace;
    
    // Touches de fonction
    constexpr unsigned int F1 = XK_F1;
    constexpr unsigned int F2 = XK_F2;
    constexpr unsigned int F3 = XK_F3;
    constexpr unsigned int F4 = XK_F4;
    constexpr unsigned int F5 = XK_F5;
    constexpr unsigned int F6 = XK_F6;
    constexpr unsigned int F7 = XK_F7;
    constexpr unsigned int F8 = XK_F8;
    constexpr unsigned int F9 = XK_F9;
    constexpr unsigned int F10 = XK_F10;
    constexpr unsigned int F11 = XK_F11;
    constexpr unsigned int F12 = XK_F12;
    
    // Chiffres
    constexpr unsigned int Num0 = XK_0;
    constexpr unsigned int Num1 = XK_1;
    constexpr unsigned int Num2 = XK_2;
    constexpr unsigned int Num3 = XK_3;
    constexpr unsigned int Num4 = XK_4;
    constexpr unsigned int Num5 = XK_5;
    constexpr unsigned int Num6 = XK_6;
    constexpr unsigned int Num7 = XK_7;
    constexpr unsigned int Num8 = XK_8;
    constexpr unsigned int Num9 = XK_9;
    
    // Lettres
    constexpr unsigned int A = XK_a;
    constexpr unsigned int B = XK_b;
    constexpr unsigned int C = XK_c;
    constexpr unsigned int D = XK_d;
    constexpr unsigned int E = XK_e;
    constexpr unsigned int F = XK_f;
    constexpr unsigned int G = XK_g;
    constexpr unsigned int H = XK_h;
    constexpr unsigned int I = XK_i;
    constexpr unsigned int J = XK_j;
    constexpr unsigned int K = XK_k;
    constexpr unsigned int L = XK_l;
    constexpr unsigned int M = XK_m;
    constexpr unsigned int N = XK_n;
    constexpr unsigned int O = XK_o;
    constexpr unsigned int P = XK_p;
    constexpr unsigned int Q = XK_q;
    constexpr unsigned int R = XK_r;
    constexpr unsigned int S = XK_s;
    constexpr unsigned int T = XK_t;
    constexpr unsigned int U = XK_u;
    constexpr unsigned int V = XK_v;
    constexpr unsigned int W = XK_w;
    constexpr unsigned int X = XK_x;
    constexpr unsigned int Y = XK_y;
    constexpr unsigned int Z = XK_z;
    
    // Pavé numérique
    constexpr unsigned int KP_Add = XK_KP_Add;
    constexpr unsigned int KP_Subtract = XK_KP_Subtract;
    constexpr unsigned int KP_Multiply = XK_KP_Multiply;
    constexpr unsigned int KP_Divide = XK_KP_Divide;
    constexpr unsigned int KP_Enter = XK_KP_Enter;
    constexpr unsigned int KP_Decimal = XK_KP_Decimal;
    constexpr unsigned int KP_0 = XK_KP_0;
    constexpr unsigned int KP_1 = XK_KP_1;
    constexpr unsigned int KP_2 = XK_KP_2;
    constexpr unsigned int KP_3 = XK_KP_3;
    constexpr unsigned int KP_4 = XK_KP_4;
    constexpr unsigned int KP_5 = XK_KP_5;
    constexpr unsigned int KP_6 = XK_KP_6;
    constexpr unsigned int KP_7 = XK_KP_7;
    constexpr unsigned int KP_8 = XK_KP_8;
    constexpr unsigned int KP_9 = XK_KP_9;
    
    // Touches de ponctuation
    constexpr unsigned int Comma = XK_comma;
    constexpr unsigned int Period = XK_period;
    constexpr unsigned int Semicolon = XK_semicolon;
    constexpr unsigned int Quote = XK_quoteright;
    constexpr unsigned int BracketLeft = XK_bracketleft;
    constexpr unsigned int BracketRight = XK_bracketright;
    constexpr unsigned int Backslash = XK_backslash;
    constexpr unsigned int Minus = XK_minus;
    constexpr unsigned int Equal = XK_equal;
    constexpr unsigned int Grave = XK_grave;
}

} // namespace input
