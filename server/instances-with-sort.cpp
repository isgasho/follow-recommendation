#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <random>

#include "picojson.h"


using namespace std;


static void die (string message)
{
	cout << "Status: 500 Internal Error" << endl << endl;
	cout << message << endl;
	exit (0);
}


int main (int argc, char ** argv)
{
	cout << "Access-Control-Allow-Origin: *" << endl;

	if (argc < 2) {
		die (string {"Too few arguments"});
	}

	string file_content;
	{
		string filename = string {"/var/lib/distsn/"} + string {argv [1]} + string {"-instances.json"};
		FILE * in = fopen (filename.c_str (), "r");
		if (in == nullptr) {
			die (string {"File "} + filename + string {" not found."});
		}
		for (; ; ) {
			char buffer [2014];
			char * reply = fgets (buffer, 1024, in);
			if (reply == nullptr) {
				break;
			}
			file_content += string {buffer};
		}
	}

	picojson::value json_value;
	string error = picojson::parse (json_value, file_content);
	if (! error.empty ()) {
		die (error);
	}

	if (! json_value.is <picojson::array> ()) {
		die (string {"Not an array"});
	}

	cout << "Content-type: application/json" << endl << endl;

	vector <picojson::value> array;
	
	for (auto item: json_value.get <picojson::array> ()) {
		array.push_back (item);
	}

	if (2 < argc) {
		if (string {argv [2]}.empty () || string {argv [2]} == string {"abc"}) {
			cout << file_content;
		} else if (string {argv [2]} == string {"zyx"}) {
			reverse (array.begin (), array.end ());

			cout << "[";
			for (unsigned int cn = 0; cn < array.size (); cn ++) {
				if (0 < cn) {
					cout << "," << endl;
				}
				cout << array.at (cn).serialize ();
			}
			cout << "]" << endl;
		} else if (string {argv [2]} == string {"shuffle"}) {
			random_device seed;
			default_random_engine engine {seed ()};
			shuffle (array.begin (), array.end (), engine);

			cout << "[";
			for (unsigned int cn = 0; cn < array.size (); cn ++) {
				if (0 < cn) {
					cout << "," << endl;
				}
				cout << array.at (cn).serialize ();
			}
			cout << "]" << endl;
		} else {
			die (string {"Bad argument: "} + string {argv [2]});
		}
	} else {
		cout << file_content;
	}
	
	return 0;
}

