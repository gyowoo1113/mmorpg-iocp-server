#pragma once

#include <windows.h>  
#include <stdio.h>  
#include <locale.h>

#include <sqlext.h>  


void load_database();
void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);