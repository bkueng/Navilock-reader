/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version 2
 *	as published by the Free Software Foundation.
 */



#ifndef GLOBAL_H_
#define GLOBAL_H_


//stl
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cmath>
using namespace std;


#include "exception.h"
#include "logging.h"


#define FOLDER_SEPARATOR '/'


string getDate(); //format: DD.MM.YY
string getTime(); //format: HH:MM:SS




/* useful string functions */

string toStr(int val);

string& toLower(string& str); //convert str to lower and return it
string toLower(const string& str);

bool cmpInsensitive(const string& str1, const string& str2);

string trim(const string& str);

string& replace(string& str, const string& find, const string& replace);



#endif /* GLOBAL_H_ */
