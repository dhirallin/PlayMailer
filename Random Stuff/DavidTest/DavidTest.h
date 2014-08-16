#pragma once

#include "resource.h"

// Virtual Keys
enum VirtualKeyCodes 
{
	VK_A = 0x41, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, VK_I, VK_J, VK_K, VK_L, 
	VK_M, VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z,
};

void PressKey(WORD vKey);
void PressKeyDown(WORD vKey, BOOL delay);
void PressKeyUp(WORD vKey, BOOL delay);