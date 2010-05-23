/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "config.h"
#include "exception.h"
#include "logging.h"



string getDate(); //format: DD.MM.YY
string getTime(); //format: HH:MM:SS


/* finds a file in folder with the substring constains in the filename. returns "" if none is found 
 * if multiple exist, the first found is returned */
string findFile(const string& folder, const string& contains, const string& extension="");



/* useful string functions */

string toStr(int val);

string& toLower(string& str); //convert str to lower and return it
string toLower(const string& str);

bool cmpInsensitive(const string& str1, const string& str2);

string trim(const string& str);

string& replace(string& str, const string& find, const string& replace);



#endif /* GLOBAL_H_ */
