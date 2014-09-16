//
//  utils.c
//  BasketballClub
//
//  Created by Angluca on 13-8-27.
//
//

#include "utils.h"
#include <assert.h>
#include "cocos2d.h"

void print_log(const char* str) {
	CCLOG("%s", str);
}

static int _HexStrToInt(char c)
{
	switch (c)
	{
		case '0':
			return 0;
			break;
		case '1':
			return 1;
			break;
		case '2':
			return 2;
			break;
		case '3':
			return 3;
			break;
		case '4':
			return 4;
			break;
		case '5':
			return 5;
			break;
		case '6':
			return 6;
			break;
		case '7':
			return 7;
			break;
		case '8':
			return 8;
			break;
		case '9':
			return 9;
			break;
		case 'a':
		case 'A':
			return 10;
			break;
		case 'b':
		case 'B':
			return 11;
			break;
		case 'c':
		case 'C':
			return 12;
			break;
		case 'd':
		case 'D':
			return 13;
			break;
		case 'e':
		case 'E':
			return 14;
			break;
		case 'f':
		case 'F':
			return 15;
			break;
		default:
			return -1;
			break;
	}
}

static unsigned char IsHexStr(const char* str, int len)
{
	for (int i = 0;i < len;i++)
	{
		if (!(((str[i] >= '0') && (str[i] <= '9'))
					|| ((str[i] >= 'A') && (str[i] <= 'F'))
                    || ((str[i] >= 'a') && (str[i] <= 'f'))))
			return 0;
	}
	return 1;
}

int HexStrToInt(const char* str)
{
    assert(str);
    const char* p = str;
    while(*p++) continue;
	int len = p - str -2;
	if (!IsHexStr(str, len))
	{
		return -1;
	}
	int num = 0;
	for (int i = 0;i < len;i++)
	{
		num = (num<<4) + _HexStrToInt(str[i]);
	}
	return num;
}

