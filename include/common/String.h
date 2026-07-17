#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

#include <stdio.h>
#include<direct.h>
#include <sys/stat.h>


namespace Festa {
	typedef char int8;
	typedef unsigned char uint8;
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef unsigned int uint;
	typedef unsigned char uchar;
	typedef long long ll;
	typedef unsigned long long ull;

	template<typename T>
	std::string toString(const T& x) {
		std::ostringstream os;
		os << x;
		return os.str();
	}

	template<typename T>
	T stringTo(const std::string& str) {
		if (str == "true")return true;
		std::stringstream ss;
		ss << str;
		T ret; ss >> ret;
		return ret;
	}

	inline std::wstring string2wstring(const std::string& str){
		int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), int(str.size()), 0, 0);
		wchar_t* buffer = new wchar_t[len + 1];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), int(str.size()), buffer, len);
		buffer[len] = '\0';
		std::wstring ret(buffer);
		delete[] buffer;
		return ret;
	}

	inline std::string wstring2string(const std::wstring& wstr){
		int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.size()), 0, 0, 0, 0);
		char* buffer = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.size()), buffer, len, 0, 0);
		buffer[len] = '\0';
		std::string ret(buffer);
		delete[] buffer;
		return ret;
	}

	inline std::string string2u8(const std::string& str){
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
		wchar_t* wbuf = new wchar_t[nwLen + 1];
		ZeroMemory(wbuf, nwLen * 2 + 2);
		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), uint(str.length()), wbuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, 0, 0, 0, 0);
		char* buf = new char[nLen + 1];
		ZeroMemory(buf, nLen + 1);
		::WideCharToMultiByte(CP_UTF8, 0, wbuf, nwLen, buf, nLen, 0, 0);

		std::string ret(buf);
		delete[]wbuf;
		delete[]buf;
		return ret;
	}

	inline std::string u82string(const std::string& str){
		int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		memset(pwBuf, 0, nwLen * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), uint(str.length()), pwBuf, nwLen);

		int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, 0, 0, 0, 0);
		char* pBuf = new char[nLen + 1];
		memset(pBuf, 0, nLen + 1);
		WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, 0, 0);

		std::string ret(pBuf);
		delete[]pBuf;
		delete[]pwBuf;
		return ret;
	}

	inline std::string base64Decode(const char* Data, size_t size){
		const char DecodeTable[] ={
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			62, // '+'
			0, 0, 0,
			63, // '/'
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
			0, 0, 0, 0, 0, 0, 0,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
			0, 0, 0, 0, 0, 0,
			26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
			39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
		};
		std::string strDecode;
		int nValue;
		size_t i = 0;
		while (i < size){
			if (*Data != '\r' && *Data != '\n'){
				nValue = DecodeTable[*Data++] << 18;
				nValue += DecodeTable[*Data++] << 12;
				strDecode += (nValue & 0x00FF0000) >> 16;
				if (*Data != '='){
					nValue += DecodeTable[*Data++] << 6;
					strDecode += (nValue & 0x0000FF00) >> 8;
					if (*Data != '='){
						nValue += DecodeTable[*Data++];
						strDecode += nValue & 0x000000FF;
					}
				}
				i += 4;
			}
			else{Data++;i++;}
		}
		return strDecode;
	}

	inline void base64Decode(char* dst, const std::string& src) {
		const char DecodeTable[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			62, // '+'
			0, 0, 0,
			63, // '/'
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
			0, 0, 0, 0, 0, 0, 0,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
			0, 0, 0, 0, 0, 0,
			26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
			39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
		};
		int nValue;
		size_t i = 0;
		const char* Data = src.c_str();
		while (i < src.size()) {
			if (*Data != '\r' && *Data != '\n') {
				nValue = DecodeTable[*Data++] << 18;
				nValue += DecodeTable[*Data++] << 12;
				(*dst++) = (nValue & 0x00FF0000) >> 16;
				if (*Data != '=') {
					nValue += DecodeTable[*Data++] << 6;
					(*dst++) = (nValue & 0x0000FF00) >> 8;
					if (*Data != '=') {
						nValue += DecodeTable[*Data++];
						(*dst++) = nValue & 0x000000FF;
					}
				}
				i += 4;
			}
			else { Data++; i++; }
		}
	}

	inline std::string base64Encode(const uchar* Data, int DataByte){
		const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string strEncode;
		unsigned char Tmp[4] = { 0 };
		int LineLength = 0;
		for (int i = 0; i < (int)(DataByte / 3); i++)
		{
			Tmp[1] = *Data++;
			Tmp[2] = *Data++;
			Tmp[3] = *Data++;
			strEncode += EncodeTable[Tmp[1] >> 2];
			strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
			strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
			strEncode += EncodeTable[Tmp[3] & 0x3F];
			if (LineLength += 4, LineLength == 76) { strEncode += "\r\n"; LineLength = 0; }
		}
		int Mod = DataByte % 3;
		if (Mod == 1)
		{
			Tmp[1] = *Data++;
			strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
			strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
			strEncode += "==";
		}
		else if (Mod == 2)
		{
			Tmp[1] = *Data++;
			Tmp[2] = *Data++;
			strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
			strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
			strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
			strEncode += "=";
		}


		return strEncode;
	}

	class String {
	public:
		std::string str;
		String() {}
		String(const char* s) :str(s) {}
		String(const std::string& s) :str(s) {}
		String(const wchar_t* ws) {
			str = wstring2string(ws);
		}
		String(const std::wstring& ws) {
			str = wstring2string(ws);
		}
		String(float number) {
			str = toString(number);
		}
		String(double number) {
			str = toString(number);
		}
		String(ll number) {
			str = toString(number);
		}
		String(ull number) {
			str = toString(number);
		}
		String(int number) {
			str = toString(number);
		}
		String(uint number) {
			str = toString(number);
		}
		friend std::ostream& operator<< (std::ostream& out, const String& str) {
			out << str.str;
			return out;
		}
		String operator+(const String& other)const {
			return String(str + other.str);
		}
		void operator+=(const String& other) {
			str += other.str;
		}
		operator std::string()const {
			return str;
		}
		ull size()const {
			return str.size();
		}
		char operator[](uint pos)const {
			return str[pos];
		}
		const std::wstring towstring()const {
			return string2wstring(str);
		}

	};



#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4



	class Logger {
	public:
		typedef std::function<void(const std::string&)> LogFunc;
		std::vector<LogFunc> tasks = { 0,0,0,0 };
		std::string file;
		Logger() {}
		Logger(const std::string& file):file(file) {
			
		}
		void addTask(LogFunc func, int level) {
			tasks[level - 1] = func;
		}
		void log(const std::string& msg, int level) {
			char color; std::string l;
			switch (level) {
			case LOG_LEVEL_DEBUG:
				color = 6, l = "DEBUG";
				break;
			case LOG_LEVEL_INFO:
				color = 2, l = "INFO";
				break;
			case LOG_LEVEL_WARNING:
				color = 3, l = "WARNING";
				break;
			case  LOG_LEVEL_ERROR:
				color = 1, l = "ERROR";
				break;
			default:
				error("Invalid log level: " + toString(level));
				return;
			}
			std::string message = l + ": " + msg;
			if (file.size()) {
				std::ofstream f(file, std::ios::out | std::ios::app);
				if (!f.is_open())
					std::cout<< coloredString("The logger file: " + file + " hasnot the access for writing.",1)<< std::endl;
				f << message << std::endl; f.close();
			}
			if (tasks[level - 1])tasks[level - 1](msg);
			else std::cout << coloredString(message, color) << std::endl;
		}
		char getColor(int level) {
			switch (level) {
			case LOG_LEVEL_DEBUG:
				return 6;
			case LOG_LEVEL_INFO:
				return 2;
			case LOG_LEVEL_WARNING:
				return 3;
			case  LOG_LEVEL_ERROR:
				return 1;
			default:
				error("Invalid log level: " + toString(level));
				return 0;
			}
			
		}
		void debug(const std::string& msg) {
			log(msg, LOG_LEVEL_DEBUG);
		}
		void info(const std::string& msg) {
			log(msg, LOG_LEVEL_INFO);
		}
		void warning(const std::string& msg) {
			log(msg, LOG_LEVEL_WARNING);
		}
		void error(const std::string& msg) {
			log(msg, LOG_LEVEL_ERROR);
		}
		static std::string coloredString(const std::string& msg, char fg, char bg = 0) {
			std::string ret = "\033[3"; ret.push_back(fg + '0');
			ret += ";1m"; ret += msg;
			ret += "\033["; ret.push_back(bg + '0');
			ret.push_back('m');
			return ret;
		}
	};

	extern Logger LOGGER;


	inline void errorlog0(const std::string& msg) {
		Logger().error(msg);
	}

#define ACCESS_MODE_EXISTS 0
#define ACCESS_MODE_WRITE 2
#define ACCESS_MODE_READ 4
#define PATH_DELIMITER '/'

	class Path {
	public:
		Path() {}
		Path(const char* path) :path(path) {
			normalize();
		}
		Path(const std::string& path) :path(path) {
			normalize();
		}
		Path(const std::wstring& path) :path(wstring2string(path)) {
			normalize();
		}
		std::string& str() {
			return path;
		}
		std::string toString()const {
			return path;
		}
		std::wstring toWstring()const {
			return string2wstring(path);
		}
		std::string toWindows()const {
			std::string p = path;
			for (char& s : p)
				if (s == PATH_DELIMITER)s = '\\';
			return p;
		}
		void clear() {
			path.clear();
		}
		ull size()const {
			return path.size();
		}
		bool access(int mode = ACCESS_MODE_EXISTS)const {
			return _waccess(string2wstring(path).c_str(), mode) == 0;
		}
		int check(int mode = ACCESS_MODE_EXISTS)const {
			if (!access(mode)) {
				LOGGER.error("The path: " + path + " doesnot exist.");
				return 1;
			}
			return 0;
		}
		bool exists()const {
			return access();
		}
		bool implemented()const {
			return path.size();
		}
		bool empty()const {
			return !path.size();
		}
		operator bool()const {
			return implemented();
		}
		void open(std::fstream& f)const {
			f.open(path);
			if (!f.is_open())LOGGER.error("Failed to open th file: " + path);
		}
		bool isFile()const {
			struct stat infos;
			stat(path.c_str(), &infos);
			return infos.st_mode & S_IFREG;
		}
		bool isDirectory()const {
			struct stat infos;
			stat(path.c_str(), &infos);
			return infos.st_mode & S_IFDIR;
		}
		Path previous()const {
			ll i;
			for (i = ll(path.size() - 1); i >= 0; i--)if (path[i] == PATH_DELIMITER)break;
			return path.substr(0, i);
		}
		Path back()const {
			ll i;
			for (i = ll(path.size() - 1); i >= 0; i--)if (path[i] == PATH_DELIMITER)break;
			return path.substr(i + 1, path.size());
		}
		std::string extension()const {
			ll i;
			for (i = ll(path.size() - 1); i >= 0; i--) {
				if (path[i] == '.')return path.substr(i + 1, path.size());
			}
			return "";
		}
		Path getDirectory()const {
			if (isDirectory())return *this;
			else return previous();
		}
		Path operator+(const Path& p)const {
			std::string tmp = path; 
			if(tmp.size())tmp.push_back(PATH_DELIMITER); 
			tmp += p.path;
			return Path(tmp);
		}
		void operator+=(const Path& p) {
			if(implemented())path.push_back(PATH_DELIMITER);
			path += p.path;
		}
		Path operator-(const Path& p)const {
			if (p.size() + 1 >= size() ||
				path[p.size()] != '/')return *this;
			for (uint i = 0; i < p.size(); i++)
				if (p.path[i] != path[i])return *this;
			return path.substr(p.size() + 1, size());
		}
		void operator-=(const Path& p) {
			*this = *this - p;
		}
		int cd()const {
			return chdir(path.c_str());
		}
		void split(std::list<std::string>& arr)const {
			arr.clear();
			arr.push_back(std::string());
			for (uint i = 0; i < path.size(); i++) {
				if (path[i] == PATH_DELIMITER)arr.push_back(std::string());
				else arr.back().push_back(path[i]);
			}
			if (arr.back().size() == 0)arr.pop_back();
		}
		int checkIsDirectory()const {
			if (isDirectory())return 0;
			LOGGER.error("The path: " + path + " is not a directory");
			return 1;
		}
		int checkIsFile()const {
			if (isFile())return 0;
			LOGGER.error("The path: " + path + " is not a file");
			return 1;
		}
		void glob(std::list<Path>& ret, const std::string& filter = "*")const {
			ret.clear();
			if (checkIsDirectory())return;
			HANDLE hFind;
			WIN32_FIND_DATA findData;
			std::wstring wstrTempDir = toWstring() + string2wstring("\\" + filter);
			hFind = FindFirstFileW(wstrTempDir.c_str(), &findData);
			if (hFind == INVALID_HANDLE_VALUE)return;
			do {
				ret.push_back(Path());
				Path t(findData.cFileName);
				if (t.toString() == "." || t.toString() == "..")ret.pop_back();
				else ret.back() = *this + t;
			} while (FindNextFile(hFind, &findData) != 0);
		}
		void globRelatively(std::list<Path>& ret, const std::string& filter = "*")const {
			ret.clear();
			if (checkIsDirectory())return;
			HANDLE hFind;
			WIN32_FIND_DATA findData;
			std::wstring wstrTempDir = toWstring() + TEXT(PATH_DELIMITER) + string2wstring(filter);
			hFind = FindFirstFile(wstrTempDir.c_str(), &findData);
			if (hFind == INVALID_HANDLE_VALUE)return;
			do {
				ret.push_back(Path());
				Path t(findData.cFileName);
				if (t.toString() == "." || t.toString() == "..")ret.pop_back();
				else ret.back() = t;
			} while (FindNextFile(hFind, &findData) != 0);
		}
		int createDirectory()const {
			if (checkIsDirectory())return 1;
			std::list<std::string> arr; split(arr);
			Path t;
			for (std::string& p : arr) {
				t.path += p;
				if (!t.exists()) {
					if (_mkdir(t.path.c_str()))return 1;
				}
				t.path.push_back(PATH_DELIMITER);
			}
			return 0;
		}
		void deleteFile()const {
			if(isFile())DeleteFile(string2wstring(path).c_str());
		}
		void createFile()const {
			if (exists())return;
			std::ofstream f(path, std::ios::out);
			f.close();
		}
		int rename(const Path& newName)const {
			return std::rename(path.c_str(), newName.toString().c_str());
		}
		int deleteDirectory()const {
			if (!exists())return 404;
			std::list<Path> arr; glob(arr);
			for (Path& p : arr) {
				if (p.isFile())p.deleteFile();
				else p.deleteDirectory();
			}
			return _rmdir(path.c_str());
		}
		void copyFile(const Path& to)const {
			CopyFile(string2wstring(path).c_str(), string2wstring(to.toWindows()).c_str(), FALSE);
		}
		void copyDirectory(const Path& to)const {
			if (!to.exists())to.createDirectory();
			std::list<Path> arr; globRelatively(arr);
			for (Path& p : arr) {
				Path tmp = *this + p;
				if (tmp.isFile())tmp.copyFile(to + p);
				else p.copyDirectory(to + p);
			}
		}
		friend std::ostream& operator<< (std::ostream& out, const Path& path) {
			out << path.path;
			return out;
		}
		operator std::string()const {
			return path;
		}
		const char* c_str()const {
			return path.c_str();
		}
		Path relativePath(const Path& p) {
			std::list<std::string> a, b;
			split(a); p.split(b);
			if (isFile())a.pop_back();
			std::list<std::string>::iterator ai=a.begin(), bi=b.begin(), aj;
			while (ai != a.end() && bi != b.end() && *ai == *bi)ai++, bi++;
			Path ret;
			aj = ai;
			while (aj != a.end())ret += "..", aj++;
			if (ret.empty())ret = ".";
			while (bi != b.end())ret += *bi, bi++;
			return ret;
		}
		static Path currentWorkDirectory() {
			static char cwd[MAX_PATH];
			cwd[0] = 0;
			char* tmp = getcwd(cwd, MAX_PATH);
			return cwd;
		}
	private:
		std::string path;
		void normalize() {
			for (char& s : path)
				if (s == '\\')s = PATH_DELIMITER;
			if (path.back() == PATH_DELIMITER)path.pop_back();
		}
	};

	class File {
	public:
		Path path;
		FILE* f = 0;
		File() {}
		File(const Path& _path, const std::string& _mode="r",bool createDirectory=true) {
			open(_path, _mode,createDirectory);
		}
		~File() {
			close();
		}
		int open(const Path& _path, const std::string& _mode = "r",bool createDirectory=true) {
			if (f)close();
			path = _path;
			f = std::fopen(path.c_str(), _mode.c_str());
			if (!_mode.size()) {
				LOGGER.error("Invalid file open mode: '" + _mode + "'");
				return 1;
			}
			mode = 0;
			switch (_mode[0]) {
			case 'r':
				mode |= 1;
				break;
			case 'w':
				mode |= 2;
				break;
			case 'a':
				mode |= 2;
				break;
			}
			if (_mode[1] == 'b')mode |= 4;
			if (_mode.back() == '+')mode |= 3;

			Path dir = path.getDirectory();
			if (createDirectory && !dir.exists() && mode) {
				dir.createDirectory();
			}

			if (!f) {
				LOGGER.error("Failed to open the file: " + path.toString() + " with mode '" + _mode + "'");
				return 1;
			}
			struct stat fs;
			stat(path.c_str(), &fs);
			fsize = fs.st_size;
			return 0;
		}
		int close() {
			if (!f)return 1;
			if (std::fclose(f)==EOF) {
				LOGGER.error("Failed to close the file: " + path.str());
				return -1;
			}
			f = 0;
			return 0;
		}
		int check()const {
			if (f)return 0;
			LOGGER.error("Failed to open the file: "+path.toString());
			return 1;
		}
		int checkReading()const {
			if (check())return 1;
			if (ableToBeRead())return 0;
			LOGGER.error("Unable to access to the file while reading: " + path.toString());
			return 1;
		}
		int checkWriting()const {
			if (check())return 1;
			if (ableToBeWrote())return 0;
			LOGGER.error("Unable to access to the file while writing: " + path.toString());
			return 1;
		}
		bool ableToBeRead()const {
			return f&&mode & 1;
		}
		bool ableToBeWrote()const {
			return f&&mode & 2;
		}
		bool isBinary()const {
			return mode & 4;
		}
		void read(std::string& s,size_t size){
			if (checkReading())return;
			s.resize(size);
			for (size_t i = 0; i < size; i++) {
				s[i] = fgetc(f);
				if (s[i] == EOF) {
					s.resize(i);
					//LOGGER.error("Reading EOF: " + path.str());
					return;
				}
			}
		}
		void readLines(std::string& s) {
			read(s, fsize-current());
		}
		void write(const char* s) {
			if (checkWriting())return;
			fputs(s, f);
		}
		void write(const std::string& s) {
			write(s.c_str());
		}
		int seek(int pos) {
			if (pos<0 || pos>fsize) {
				LOGGER.error("Invalid seek position: " + toString(pos));
				return 1;
			}
			fseek(f, pos, SEEK_SET);
			return 0;
		}
		int current() {
			return ftell(f);
		}
		int end()const {
			return fsize;
		}
		int size()const {
			return fsize;
		}
		int readBinaryData(void* data, size_t size) {
			if (checkReading())return 1;
			for (size_t i = 0; i < size; i++) {
				((char*)data)[i] = fgetc(f);
			}
			return 0;
		}
		template<typename T>
		int readBinaryData(T& data) {
			if (checkReading())return 1;
			char* ptr = (char*)&data;
			for (size_t i = 0; i < sizeof(T); i++) {
				ptr[i] = fgetc(f);
			}
			return 0;
		}
		template<typename T>
		File& operator>>(T& data) {
			readBinaryData(data);
			return *this;
		}
		template<typename T>
		int writeData(const T& data) {
			if (checkWriting())return 1;
			write(toString(data));
			return 0;
		}
		int writeBinaryData(void* data, size_t size) {
			if (checkWriting())return 1;
			for (size_t i = 0; i < size; i++)fputc(((char*)data)[i], f);
			return 0;
		}
		template<typename T>
		int writeBinaryData(const T& data) {
			if (checkWriting())return 1;
			char* ptr = (char*)&data;
			for (size_t i = 0; i < sizeof(T); i++)fputc(ptr[i], f);
			return 0;
		}
		template<typename T>
		File& operator<<(const T& data) {
			if (checkWriting())return *this;
			/*if (isBinary()) {
				char* ptr = (char*)&data;
				for (size_t i = 0; i < sizeof(T); i++)fputc(ptr[i], f);
			}
			else {
				write(toString(data));
			}*/
			char* ptr = (char*)&data;
			for (size_t i = 0; i < sizeof(T); i++)fputc(ptr[i], f);
			return *this;
		}
	private:
		uchar mode = 0;
		int fsize = 0;
	};

}


