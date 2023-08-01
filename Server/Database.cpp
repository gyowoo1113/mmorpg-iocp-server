#include "pch.h"
#include "Database.h"

#define NAME_LEN 20

void loadDatabase()
{
	setlocale(LC_ALL, "korean");

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szName[NAME_LEN];
	SQLINTEGER user_x, user_y, user_level, user_exp, user_hp;
	SQLLEN cbName = 0, cbuser_x = 0, cbuser_y = 0, cbuser_level, cbuser_exp, cbuser_hp;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"gs-termproject", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT user_name, user_x, user_y, user_level, user_exp, user_hp FROM user_table ORDER BY 1, 2, 3, 4, 5, 6 ", SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

						retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, szName, NAME_LEN, &cbName);
						retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_x, 10, &cbuser_x);
						retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_y, 10, &cbuser_y);
						retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_level, 10, &cbuser_level);
						retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_exp, 10, &cbuser_exp);
						retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &user_hp, 10, &cbuser_hp);

						for (int i = 0; ; i++) {
							retcode = SQLFetch(hstmt);
							if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
								showError(hstmt, SQL_HANDLE_STMT, retcode);
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
							{
								char* str;
								int size_s = WideCharToMultiByte(CP_ACP, 0, szName, -1, NULL, 0, NULL, NULL);
								str = new char[size_s];
								WideCharToMultiByte(CP_ACP, 0, szName, -1, str, size_s, 0, 0);

								USER_DATA user;
								user.x = user_x;
								user.y = user_y;
								user.level = user_level;
								user.exp = user_exp;
								user.hp = user_hp;

								strcpy_s(user.name, str);

								//cout << "x,y:" << user_x <<","<< user_y << " level,exp: " << user_level << "," << user_exp << "hp:" << user_hp << " name:" << str;
								//cout << endl;
								g_db_users.push_back(user);
							}
							else
								break;
						}
					}
					else
					{
						showError(hstmt, SQL_HANDLE_STMT, retcode);
					}

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}

void updateDatabase(int id)
{
	setlocale(LC_ALL, "korean");

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR words[1024];

	USER_DATA user;
	user.level = clients[id]._level;
	user.exp = clients[id]._exp;
	user.hp = clients[id]._hp;

	SQLWCHAR* str;
	int size_s = MultiByteToWideChar(CP_ACP, 0, clients[id]._name, -1, NULL, NULL); str = new SQLWCHAR[size_s];
	MultiByteToWideChar(CP_ACP, 0, clients[id]._name, strlen(clients[id]._name) + 1, str, size_s);
	wsprintf(words, L"UPDATE user_table SET user_x = %d, user_y = %d, user_level = %d, user_exp = %d, user_hp = %d WHERE user_name = \'%s\'",
		clients[id].x, clients[id].y, user.level, user.exp , user.hp, str);

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"gs-termproject", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)words, SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

					}
					else
					{
						showError(hstmt, SQL_HANDLE_STMT, retcode);
					}

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}

void showError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void insertToDatabase(char* name, short x, short y, int level, int exp, int hp)
{
	setlocale(LC_ALL, "korean");

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR words[1024];

	SQLWCHAR* str;
	int size_s = MultiByteToWideChar(CP_ACP, 0, name, -1, NULL, NULL); str = new SQLWCHAR[size_s];
	MultiByteToWideChar(CP_ACP, 0, name, strlen(name) + 1, str, size_s);
	wsprintf(words, L"INSERT INTO user_table (user_name, user_x, user_y , user_level, user_exp, user_hp) VALUES (\'%s\',%d,%d,%d,%d,%d)",
		str, x, y, level, exp, hp);

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"gs-termproject", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)words, SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						
						USER_DATA insert_data;
						insert_data.exp = exp;
						insert_data.hp = hp;
						insert_data.level = level;
						insert_data.x = x;
						insert_data.y = y;
						strcpy_s(insert_data.name, name);

						g_db_users.emplace_back(insert_data);
					}
					else
					{
						showError(hstmt, SQL_HANDLE_STMT, retcode);
					}

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}
