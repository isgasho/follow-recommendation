#ifndef DISTSN_H
#define DISTSN_H


#include <set>
#include <vector>
#include <string>
#include "picojson.h"


class ExceptionWithLineNumber: public std::exception {
public:
	unsigned int line;
public:
	ExceptionWithLineNumber () {
		line = 0;
	};
	ExceptionWithLineNumber (unsigned int a_line) {
		line = a_line;
	};
};


class HttpException: public ExceptionWithLineNumber {
public:
	HttpException (unsigned int a_line): ExceptionWithLineNumber (a_line) { };
	HttpException () { };
};


class HostException: public ExceptionWithLineNumber {
public:
	HostException (unsigned int a_line): ExceptionWithLineNumber (a_line) { };
	HostException () { };
};


class TootException: public ExceptionWithLineNumber {
public:
	TootException (unsigned int a_line): ExceptionWithLineNumber (a_line) { };
	TootException () { };
};


class UserException: public std::exception {
	/* Nothing. */
};


class HttpGlobal {
public:
	HttpGlobal ();
};


class Http {
public:
	static HttpGlobal global;
	CURL *curl;
public:
	Http ();
	std::string perform (std::string url, std::vector <std::string> header);
	std::string perform (std::string url);
	std::string endure (std::string url);
	~Http ();
};


std::string get_id (const picojson::value &toot);
std::vector <picojson::value> get_timeline (std::string host);
std::vector <picojson::value> get_timeline (std::string host, unsigned int time_depth);
time_t get_time (const picojson::value &toot);
time_t str2time (std::string s);
std::string get_host_title (std::string domain);
std::string get_host_title (std::string domain, Http &http);
std::string get_host_thumbnail (std::string domain);
std::string get_host_thumbnail (std::string domain, Http &http);
std::set <std::string> get_international_hosts ();


#endif /* #ifndef DISTSN_H */

