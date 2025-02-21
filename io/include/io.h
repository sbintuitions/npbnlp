#ifndef NLPBP_IO_H
#define NLPBP_IO_H

#include<cstdlib>
#include<cstring>
#include<string>
#include<memory>
#include<vector>
#include<iostream>
#include<fstream>
namespace npbnlp {
	class io {
		public:
			static unsigned int u8size(const char  *c) {
			       	return "\1\1\1\1\1\1\1\1\1\1\1\1\2\2\3\4"[(*c & 0xFF) >> 4];
				/*
				const unsigned char uc = *c;
				if (uc <= 0x7F)
					return 1;
				else if (uc <= 0xDF)
					return 2;
				else if (uc <= 0xEF)
					return 3;
				else if (uc <= 0xF7)
					return 4;
				throw "undefined character in utf-8";
				*/
			}
			static unsigned int u8strlen(const char *c) {
				unsigned int shift = 0;
				unsigned int count = 0;
				const char *h = c;
				try {
					while ((shift = u8size(h)) && *h != '\0') {
						h += shift;
						++count;
					}
				} catch (const char *ex) {
					throw ex;
				}
				return count;
			}
			static unsigned int c2i(const char *str, int size) {
				if (size >= 3 && (str[0] & 0xF0) == 0xE0) {
					return (unsigned int)((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | ((str[2] & 0x3F));
				} else if ((unsigned char)(str[0]) < 0x80) {
					return (unsigned int)str[0];
				} else if (size >= 2 && (str[0] & 0xE0) == 0xC0) {
					return (unsigned int)(((str[0] & 0x1F) << 6) | (str[1] & 0x3F)) ;
				} else if (size >= 4 && (str[0] & 0xF8) == 0xF0) {
					return (unsigned int)(((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F));
				}
				/*
				unsigned int w = 0;
				unsigned char *c = (unsigned char*)&w; 
				for (int i = 0; i < size; ++i) {
					*(c+i) = (const unsigned char)*(str+i);
				}
				return w;
				*/
			}
			static int i2c(unsigned int c, char *buf) {
				if (c == 0) {
					buf[0] = '\0';
					return 0;
				} else if (c <= 0x7F) {
					*buf = (char)c;
					return 1;
				} else if (c <= 0x7FF) {
					buf[1] = 0x80 | (c & 0x3F);
					c >>= 6;
					buf[0] = 0xC0 | c;
					return 2;
				} else if (c <= 0xFFFF) {
					buf[2] = 0x80 | (c & 0x3F);
					c >>= 6;
					buf[1] = 0x80 | (c & 0x3F);
					c >>= 6;
					buf[0] = 0xE0 | c;
					return 3;
				} else {
					buf[3] = 0x80 | (c & 0x3F);
					c >>= 6;
					buf[2] = 0x80 | (c & 0x3F);
					c >>= 6;
					buf[1] = 0x80 | (c & 0x3F);
					c >>= 6;
					buf[0] = 0xF0 | c;
					return 4;
				}
				/*
				int size = io::u8size((char*)&c);
				unsigned char *w = (unsigned char*)&c;
				for (int i = 0; i < size; ++i) {
					*(buf+i) = *(w+i);
				}
				*(buf+size) = '\0';
				return size;
				*/
			}
			static void chomp(char *str) {
				int len = std::strlen(str);
				while (*(str+len-1) == '\n' || *(str+len-1) == '\r') {
					*(str+len-1) = '\0';
					--len;
				}
			}
			static void chomp(std::string& str) {
				int len = str.size();
				while (str[len-1] == '\n' || str[len-1] == '\r') {
					str[len-1] = '\0';
					str.pop_back();
					--len;
				}
			}
			static void swap_cr2ws(char *str) {
				size_t len = std::strlen(str);
				while (*(str+len-1) == '\n' || *(str+len-1) == '\r') {
					*(str+len-1) = ' ';
					--len;
				}
				while (len+1 < std::strlen(str) && *(str+len+1) == ' ') {
					*(str+len+1) = '\0';
					++len;
				}
			}
			static void swap_cr2ws(std::string& str) {
				int len = str.size();
				while (str[len-1] == '\n' || str[len-1] == '\r') {
					str[len-1] = ' ';
					--len;
				}
				while (len+1 < (int)str.size() && str[len+1] == ' ') {
					str[len+1] = '\0';
					++len;
				}
			}
			static void s2i(const char *str, std::vector<unsigned int>& v) {
				unsigned int shift = 0;
				const char *h = str;
				try {
					while ((shift = u8size(h)) && *h != '\0') {
						unsigned int c = c2i(h, shift);
						v.push_back(c);
						h += shift;
					}
				} catch (const char *ex) {
					throw ex;
				}
			}
			io();
			io(io&& f);
			io(const io& f);
			io(const char *f);
			io(std::istream& in);
			io& operator=(const io& f);
			io& operator=(io&& f) noexcept;
			virtual ~io();
			std::shared_ptr<std::vector<unsigned int> > raw;
			std::vector<int> head;
		private:
	};
}

#endif
