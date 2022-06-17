#pragma once

#include <windows.h>  
#include <stdio.h>  
#include <locale.h>

#include <sqlext.h>  


void load_database();
void update_database(int id);
void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
void insertToDatabase(char* name, short x, short y, int level, int exp, int hp);