#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <Windows.h>

#include "simconio.h"

#define CONOUT GetStdHandle(STD_OUTPUT_HANDLE)

COORD GetConsoleCursorPosition();
void draw_text(COORD ,char[], UINT, UINT, UINT, WORD, char);

COORD GetConsoleCursorPosition(){
	CONSOLE_SCREEN_BUFFER_INFO binfo;
	GetConsoleScreenBufferInfo(CONOUT, &binfo);
	return binfo.dwCursorPosition;
}

SIMCON int scanf_vs(const char * format, ...){
	int ret;
	va_list args;
	va_start(args, format);
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	ret = vscanf_s(format, args);
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	va_end(args);
	return ret;
}

SIMCON void _pause(const char * format, ...){
	_putch('\n');
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	getch_s();
}

SIMCON void pause(){
	_pause("Press any key to continue.\n");
}

SIMCON CHS getch_s(){
	CHS ret = { 0, 0 };
	ret.f = _getch();
	if (ret.f == 0 || ret.f == 0xE0) ret.s = _getch();
	return ret;
}

SIMCON _Bool getstr(char string[], UINT size, char mask, _Bool escape){
	CONSOLE_CURSOR_INFO origci, newci;
	GetConsoleCursorInfo(CONOUT, &origci);
	newci.bVisible = 1;
	newci.dwSize = 25;
	SetConsoleCursorInfo(CONOUT, &newci);
	COORD cur = GetConsoleCursorPosition(), temp;
	temp.Y = cur.Y;
	_Bool insert = 0;
	memset(string + strlen(string), 0, size - strlen(string));
	for (int i = 0; i < strlen(string); i++)_putch(mask ? mask : string[i]);
	UINT pos = strlen(string);
	_Bool ret = 1;
	while (1){
		int c = _getch();
		if (c == 0x0D) break;
		else if (c == 0x1B && escape){
			ret = 0;
			break;
		}
		else if (c == 0 || c == 0xE0){
			int k = _getch();
			if (k == 0x52){
				insert = !insert;
				newci.dwSize = insert ? 50 : 25;
				SetConsoleCursorInfo(CONOUT, &newci);
			}
			else if (k == 0x4B && pos > 0){
				pos -= 1;
				temp.X = cur.X + pos;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x4D && pos < strlen(string)){
				pos += 1;
				temp.X = cur.X + pos;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x53 && pos < strlen(string)){
				memmove(string + pos, string + pos + 1, size - pos - 1);
				for (int i = pos; i < strlen(string); i++)_putch(mask ? mask : string[i]);
				_putch(' ');
				temp.X = cur.X + pos;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x53 && pos == strlen(string) && strlen(string) != 0){
				memmove(string + pos - 1, string + pos, size - pos);
				pos -= 1;
				temp.X = cur.X + pos;
				SetConsoleCursorPosition(CONOUT, temp);
				for (int i = pos; i < strlen(string); i++)_putch(mask ? mask : string[i]);
				_putch(' ');
				temp.X = cur.X + pos;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else _putch('\a');
		}
		else if (c == 0x08 && pos > 0 && strlen(string) != 0){
			memmove(string + pos - 1, string + pos, size - pos);
			pos -= 1;
			temp.X = cur.X + pos;
			SetConsoleCursorPosition(CONOUT, temp);
			for (int i = pos; i < strlen(string); i++)_putch(mask ? mask : string[i]);
			_putch(' ');
			temp.X = cur.X + pos;
			SetConsoleCursorPosition(CONOUT, temp);
		}
		else if (((c >= '!' && c <= '~') || (c == ' ' && !mask)) && strlen(string) < (size - 1)) {
			if (insert){
				string[pos] = c;
				_putch(c);
				pos += 1;
			}
			else {
				memmove(string + pos + 1, string + pos, size - pos - 1);
				string[pos] = c;
				for (int i = pos; i < strlen(string); i++)_putch(mask ? mask : string[i]);
				pos += 1;
				temp.X = cur.X + pos;
				SetConsoleCursorPosition(CONOUT, temp);
			}
		}
		else _putch('\a');
	}
	temp.X = 0;
	temp.Y += 1;
	SetConsoleCursorPosition(CONOUT, temp);
	SetConsoleCursorInfo(CONOUT, &origci);
	return ret;
}

void draw_text(COORD pos, char str[], UINT len, UINT index, UINT r_index, WORD o_attr, char mask){
	int t;
	SetConsoleCursorPosition(CONOUT, pos);
	o_attr = (o_attr | COMMON_LVB_UNDERSCORE | COMMON_LVB_GRID_HORIZONTAL);
	WORD attr;
	UINT start = index - r_index;
	for (int i = 0; i < len; i++){
		attr = o_attr;
		if (i == 0)attr |= COMMON_LVB_GRID_LVERTICAL;
		if (i == len - 1)attr |= COMMON_LVB_GRID_RVERTICAL;
		SetConsoleTextAttribute(CONOUT, attr);
		if (str[start + i] != 0 && (start + i) < strlen(str)) _putch(mask ? mask : str[start + i]);
		else _putch(' ');
	}
	SetConsoleCursorPosition(CONOUT, (COORD){ pos.X + r_index, pos.Y});
}

SIMCON int getstr_ex(COORD pos, char string[], char mask, UINT size, UINT length, char attribute){
	_Bool escape = (attribute & SCI_ALLOW_ESCAPE), tab = (attribute & SCI_ALLOW_TAB);
	CONSOLE_CURSOR_INFO origci, newci;
	GetConsoleCursorInfo(CONOUT, &origci);
	newci.bVisible = 1;
	newci.dwSize = 25;
	SetConsoleCursorInfo(CONOUT, &newci);
	CONSOLE_SCREEN_BUFFER_INFO bi;
	GetConsoleScreenBufferInfo(CONOUT, &bi);
	UINT index = strlen(string), r_index;
	if (index > length - 1) r_index = length - 1;
	else r_index = index;
	_Bool insert = 0;
	memset(string + strlen(string), 0, size - strlen(string));
	draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
	_Bool ret = SCI_STATE_SUCCESS;
	while (1){
		int c = _getch();
		if (c == 0x0D) break;
		else if (c == 0x1B && escape){
			ret = SCI_STATE_CANCELLED;
			break;
		}
		else if (c == 0 || c == 0xE0){
			int k = _getch();
			if (k == 0x52){
				insert = !insert;
				newci.dwSize = insert ? 50 : 25;
				SetConsoleCursorInfo(CONOUT, &newci);
			}
			else if (k == 0x4B && index > 0){
				index -= 1;
				if (r_index > 0) r_index -= 1;
				draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
			}
			else if (k == 0x4D && index < strlen(string)){
				index += 1;
				if (r_index < length - 1) r_index += 1;
				draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
			}
			else if (k == 0x53 && index < strlen(string)){
				memmove(string + index, string + index + 1, size - index - 1);
				draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
			}
			else if (k == 0x53 && index == strlen(string) && strlen(string) != 0){
				memmove(string + index - 1, string + index, size - index);
				index -= 1;
				if (r_index > 0) r_index -= 1;
				if ((index - r_index) > 0 && r_index == 0) r_index = 1;
				draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
			}
			else _putch('\a');
		}
		else if (c == 0x08 && index > 0 && strlen(string) != 0){
			memmove(string + index - 1, string + index, size - index);
			index -= 1;
			if (r_index > 0) r_index -= 1;
			if ((index - r_index) > 0 && r_index == 0) r_index = 1;
			draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
		}
		else if (((c >= '!' && c <= '~') || (c == ' ' && !mask)) && strlen(string) < (size - 1)) {
			if (insert){
				string[index] = c;
				index += 1;
				if (r_index < length - 1) r_index += 1;
				draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
			}
			else {
				memmove(string + index + 1, string + index, size - index - 1);
				string[index] = c;
				index += 1;
				if (r_index < length - 1) r_index += 1;
				draw_text(pos, string, length, index, r_index, bi.wAttributes, mask);
			}
		}
		else _putch('\a');
	}
	SetConsoleTextAttribute(CONOUT, bi.wAttributes);
	SetConsoleCursorInfo(CONOUT, &origci);
	return ret;
}

SIMCON int getint(unsigned int digit, _Bool escape){
	CONSOLE_CURSOR_INFO origci, newci;
	GetConsoleCursorInfo(CONOUT, &origci);
	newci.bVisible = 1;
	newci.dwSize = 100;
	SetConsoleCursorInfo(CONOUT, &newci);
	COORD cur = GetConsoleCursorPosition(), temp;
	temp.Y = cur.Y;
	char* cache = malloc(digit + 1);
	int ret = 0;
	memset(cache, '0', digit);
	cache[digit] = 0;
	printf("%s\b", cache);
	int index = digit - 1;
	int c;
	while (1) {
		c = _getch();
		if (c == 0x0D) break;
		if (c == 0x1B){
			ret = -1;
			break;
		}
		if (c == 0xE0){
			int k = _getch();
			if (k == 0x4B && index > 0){
				index -= 1;
				temp.X = cur.X + index;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x4D && index < digit - 1){
				index += 1;
				temp.X = cur.X + index;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x48 || k == 0x50){
				int n = cache[index] - '0';
				if (k == 0x48) n += 1;
				else n -= 1;
				n %= 10;
				if (n < 0) n += 10;
				cache[index] = '0' + n;
				printf("%c\b", cache[index]);
			}
		}
		if (c >= '0' && c <= '9'){
			if (cache[0] == '0' && index > 0){
				int i = index;
				memmove(cache, cache + 1, index);
				SetConsoleCursorPosition(CONOUT, cur);
				printf("%s", cache);
				temp.X = cur.X + index;
				SetConsoleCursorPosition(CONOUT, temp);
			}
			cache[index] = c;
			printf("%c\b", cache[index]);
		}
		if (c == 0x08 && index > 0){
			memmove(cache + 1, cache, index);
			cache[0] = '0';
			SetConsoleCursorPosition(CONOUT, cur);
			printf("%s", cache);
			temp.X = cur.X + index;
			SetConsoleCursorPosition(CONOUT, temp);
		}
	}
	cache[digit] = 0;
	cur.X = 0;
	cur.Y += 1;
	SetConsoleCursorPosition(CONOUT, cur);
	if (ret >= 0) ret = atoi(cache);
	free(cache);
	SetConsoleCursorInfo(CONOUT, &origci);
	return ret;
}

SIMCON _Bool confirm(const char * format, ...){
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("(y/n)");
	CHS c;
	while (1){
		c = getch_s();
		switch (c.f){
		case 'y':case 'Y': case 'n':case 'N':
			printf("%s\n", (c.f == 'y' || c.f == 'Y') ? "Yes" : "No");
			return (c.f == 'y' || c.f == 'Y');
		default: _putch('\a');
		}
	}
}

SIMCON _Bool getdate(struct tm *value, _Bool escape){
	CONSOLE_CURSOR_INFO origci, newci;
	GetConsoleCursorInfo(CONOUT, &origci);
	newci.bVisible = 1;
	newci.dwSize = 100;
	SetConsoleCursorInfo(CONOUT, &newci);
	struct tm cache;
	memmove(&cache, value, sizeof(struct tm));
	COORD temp, cur = GetConsoleCursorPosition();
	temp.X = cur.X;
	temp.Y = cur.Y;
	int index = 0, devs[8] = { 0, 1, 2, 3, 5, 6, 8, 9 };
	char disp[11];
	strftime(disp, 11, "%Y-%m-%d", value);
	printf(disp);
	SetConsoleCursorPosition(CONOUT, temp);
	_Bool ret = 1;
	int c;
	while (1){
		c = _getch();
		if (c == 0x0D) break;
		if (c == 0x1B){
			ret = 0;
			break;
		}
		if (c == 0xE0){
			int k = _getch();
			if (k == 0x4B && index > 0){
				index -= 1;
				temp.X = cur.X + devs[index];
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x4D && index < 7){
				index += 1;
				temp.X = cur.X + devs[index];
				SetConsoleCursorPosition(CONOUT, temp);
			}
			else if (k == 0x48 || k == 0x50){
				int year, mon;
				sscanf_s(disp, "%4d-%2d", &year, &mon);
				_Bool big = (mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12);
				int n = disp[devs[index]] - '0';
				if (k == 0x48) n += 1;
				else n -= 1;
				if (index < 4){
					n %= 10;
					if (n < 0) n += 10;
					disp[devs[index]] = '0' + n;
					sscanf_s(disp, "%4d", &year);
					if (year % 4 != 0 && mon == 2 && disp[devs[6]] == '2' && disp[devs[7]] > '8') disp[devs[7]] = '8';
				}
				else if (index < 6){
					if (index == 4){
						n %= 2;
						if (n < 0) n += 2;
						if (n == 1 && disp[devs[5]] > '2') disp[devs[5]] = '2';
						else if (n == 0 && disp[devs[5]] < '1') disp[devs[5]] = '1';
					}
					else if (index == 5){
						if (disp[devs[4]] == '0'){
							n -= 1;
							n %= 9;
							if (n < 0) n += 9;
							n += 1;
						}
						else {
							n %= 3;
							if (n < 0) n += 3;
						}
					}
					disp[devs[index]] = '0' + n;
					sscanf_s(disp, "%*4d-%2d", &mon);
					big = (mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12);
					if (mon == 2 && disp[devs[6]] == '3') disp[devs[6]] = '2';
					if (year % 4 != 0 && mon == 2 && disp[devs[6]] == '2' && disp[devs[7]] > '9') disp[devs[7]] = '8';
					else if (disp[devs[6]] == '3' && disp[devs[7]] > '0') disp[devs[7]] = big ? '1' : '0';
				}
				else {
					if (index == 6){
						if (mon == 2){
							n %= 3;
							if (n < 0) n += 3;
							if (n == 2 && disp[devs[7]] > '8' && year % 4 != 0) disp[devs[7]] = '8';
							else if (n == 0 && disp[devs[7]] < '1') disp[devs[7]] = '1';
						}
						else {
							n %= 4;
							if (n < 0) n += 4;
							if (n == 3 && disp[devs[7]] > '0') disp[devs[7]] = big ? '1' : '0';
							else if (n == 0 && disp[devs[7]] < '1') disp[devs[7]] = '1';
						}
					}
					else {
						if (disp[devs[6]] == '0'){
							n -= 1;
							n %= 9;
							if (n < 0) n += 9;
							n += 1;
						}
						else if (mon == 2 && disp[devs[6]] == '2' && year % 4 != 0){
							n %= 9;
							if (n < 0) n += 9;
						}
						else if (disp[devs[6]] == '3') {
							if (!big) n = 0;
							else {
								n %= 2;
								if (n < 0) n += 2;
							}
						}
						else {
							n %= 10;
							if (n < 0) n += 10;
						}
					}
					disp[devs[index]] = '0' + n;
				}
				SetConsoleCursorPosition(CONOUT, cur);
				printf(disp);
				SetConsoleCursorPosition(CONOUT, temp);
			}
		}
		if (c >= '0' && c <= '9'){
			int year, mon;
			sscanf_s(disp, "%4d-%2d", &year, &mon);
			_Bool big = (mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12);
			if (index < 4){
				disp[devs[index]] = c;
				sscanf_s(disp, "%4d", &year);
				if (year % 4 != 0 && mon == 2 && disp[devs[6]] == '2' && disp[devs[7]] > '8') disp[devs[7]] = '8';
			}
			else if (index < 6){
				if (index == 4){
					if (c != '0' && c != '1') continue;
				}
				else {
					if (disp[devs[4]] == '0' && c == '0') continue;
					if (disp[devs[4]] == '1' && c > '2') continue;
				}
				disp[devs[index]] = c;
				sscanf_s(disp, "%*4d-%2d", &mon);
				big = (mon == 1 || mon == 3 || mon == 5 || mon == 7 || mon == 8 || mon == 10 || mon == 12);
				if (mon == 2 && disp[devs[6]] == '3') disp[devs[6]] = '2';
				if (year % 4 != 0 && mon == 2 && disp[devs[6]] == '2' && disp[devs[7]] > '9') disp[devs[7]] = '8';
				else if (disp[devs[6]] == '3' && disp[devs[7]] > '0') disp[devs[7]] = big ? '1' : '0';
			}
			else{
				if (index == 6){
					if (mon == 2){
						if (c > '2') continue;
						if (c == '2' && disp[devs[7]] > '8' && year % 4 != 0) disp[devs[7]] = '8';
						else if (c == '0' && disp[devs[7]] < '1') disp[devs[7]] = '1';
					}
					else {
						if (c > '3') continue;
						if (c == '3' && disp[devs[7]] > '0') disp[devs[7]] = big ? '1' : '0';
						else if (c == '0' && disp[devs[7]] < '1') disp[devs[7]] = '1';
					}
				}
				else {
					if (disp[devs[6]] == '0' && c < '1') continue;
					if (mon == 2 && disp[devs[6]] == '2' && year % 4 != 0 && c > '8') continue;
					if (disp[devs[6]] == '3' && !big && c != '0') continue;
					if (disp[devs[6]] == '3' && big && c > '1') continue;
				}
				disp[devs[index]] = c;
			}
			SetConsoleCursorPosition(CONOUT, cur);
			printf(disp);
			if (index < 7){
				index += 1;
				temp.X = cur.X + devs[index];
			}
			SetConsoleCursorPosition(CONOUT, temp);
		}
	}
	sscanf_s(disp, "%4d-%2d-%2d", &cache.tm_year, &cache.tm_mon, &cache.tm_mday);
	cache.tm_year -= 1900;
	cache.tm_mon -= 1;
	memmove(value, &cache, sizeof(struct tm));
	temp.X = 0;
	temp.Y += 1;
	SetConsoleCursorPosition(CONOUT, temp);
	SetConsoleCursorInfo(CONOUT, &origci);
	return ret;
}