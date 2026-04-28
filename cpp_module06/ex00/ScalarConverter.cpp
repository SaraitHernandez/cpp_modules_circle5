/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScalarConverter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sarherna <sarherna@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 00:00:00 by sarherna            #+#    #+#             */
/*   Updated: 2026/04/25 00:00:00 by sarherna           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ScalarConverter.hpp"
#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {

enum Type { TYPE_CHAR, TYPE_INT, TYPE_FLOAT, TYPE_DOUBLE, TYPE_INVALID };

bool isPseudoFloat(const std::string& s) {
	return (s == "nanf" || s == "+inff" || s == "-inff");
}

bool isPseudoDouble(const std::string& s) {
	return (s == "nan" || s == "+inf" || s == "-inf");
}

bool isIntegerString(const std::string& s) {
	if (s.empty())
		return false;
	size_t i = 0;
	if (s[i] == '+' || s[i] == '-')
		++i;
	if (i == s.size())
		return false;
	for (; i < s.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(s[i])))
			return false;
	}
	return true;
}

bool parseInt(const std::string& s, int& out) {
	if (!isIntegerString(s))
		return false;
	errno = 0;
	char* end = 0;
	long v = std::strtol(s.c_str(), &end, 10);
	if (end == s.c_str() || *end != '\0' || errno == ERANGE)
		return false;
	if (v > INT_MAX || v < INT_MIN)
		return false;
	out = static_cast<int>(v);
	return true;
}

bool parseDoubleFull(const std::string& s, double& out) {
	errno = 0;
	char* end = 0;
	double v = std::strtod(s.c_str(), &end);
	if (end == s.c_str() || *end != '\0' || errno == ERANGE)
		return false;
	out = v;
	return true;
}

bool parseFloatLiteral(const std::string& s, float& out) {
	if (s.empty() || s[s.size() - 1] != 'f')
		return false;
	std::string core = s.substr(0, s.size() - 1);
	if (core.empty())
		return false;
	errno = 0;
	char* end = 0;
	double v = std::strtod(core.c_str(), &end);
	if (end == core.c_str() || *end != '\0' || errno == ERANGE)
		return false;
	out = static_cast<float>(v);
	return true;
}

Type detectType(const std::string& s, char& c, int& i, float& f, double& d) {
	if (isPseudoFloat(s)) {
		if (s == "nanf")
			f = static_cast<float>(NAN);
		else if (s == "+inff")
			f = static_cast<float>(INFINITY);
		else
			f = static_cast<float>(-INFINITY);
		return TYPE_FLOAT;
	}
	if (isPseudoDouble(s)) {
		if (s == "nan")
			d = NAN;
		else if (s == "+inf")
			d = INFINITY;
		else
			d = -INFINITY;
		return TYPE_DOUBLE;
	}
	if (s.size() >= 2 && s[s.size() - 1] == 'f') {
		if (parseFloatLiteral(s, f))
			return TYPE_FLOAT;
	}
	if (isIntegerString(s) && parseInt(s, i))
		return TYPE_INT;
	if (parseDoubleFull(s, d))
		return TYPE_DOUBLE;
	if (s.size() == 3 && s[0] == '\'' && s[2] == '\'') {
		c = s[1];
		return TYPE_CHAR;
	}
	if (s.size() == 1) {
		c = s[0];
		return TYPE_CHAR;
	}
	return TYPE_INVALID;
}

void printCharFromDouble(double val) {
	if (std::isnan(val) || std::isinf(val)) {
		std::cout << "char: impossible" << std::endl;
		return;
	}
	if (val < static_cast<double>(CHAR_MIN) || val > static_cast<double>(CHAR_MAX)) {
		std::cout << "char: impossible" << std::endl;
		return;
	}
	char ch = static_cast<char>(static_cast<int>(val));
	unsigned char uch = static_cast<unsigned char>(ch);
	if (!std::isprint(uch))
		std::cout << "char: Non displayable" << std::endl;
	else
		std::cout << "char: '" << ch << "'" << std::endl;
}

void printIntFromDouble(double val) {
	if (std::isnan(val) || std::isinf(val)) {
		std::cout << "int: impossible" << std::endl;
		return;
	}
	if (val < static_cast<double>(INT_MIN) || val > static_cast<double>(INT_MAX)) {
		std::cout << "int: impossible" << std::endl;
		return;
	}
	int iv = static_cast<int>(val);
	if (static_cast<double>(iv) != val) {
		std::cout << "int: impossible" << std::endl;
		return;
	}
	std::cout << "int: " << iv << std::endl;
}

std::string formatFloat(float val) {
	std::ostringstream os;
	if (std::isnan(val))
		return "nanf";
	if (std::isinf(val))
		return (val < 0 ? "-inff" : "+inff");
	os << std::fixed << std::setprecision(1) << val << "f";
	return os.str();
}

std::string formatDouble(double val) {
	std::ostringstream os;
	if (std::isnan(val))
		return "nan";
	if (std::isinf(val))
		return (val < 0 ? "-inf" : "+inf");
	os << std::fixed << std::setprecision(1) << val;
	return os.str();
}

void printFloatFromDouble(double val) {
	float fv = static_cast<float>(val);
	std::cout << "float: " << formatFloat(fv) << std::endl;
}

void printDoubleFromDouble(double val) {
	std::cout << "double: " << formatDouble(val) << std::endl;
}

double toDouble(Type t, char c, int i, float f, double d) {
	if (t == TYPE_CHAR)
		return static_cast<double>(static_cast<unsigned char>(c));
	if (t == TYPE_INT)
		return static_cast<double>(i);
	if (t == TYPE_FLOAT)
		return static_cast<double>(f);
	return d;
}

} // namespace

ScalarConverter::ScalarConverter() {}

ScalarConverter::ScalarConverter(const ScalarConverter&) {}

ScalarConverter& ScalarConverter::operator=(const ScalarConverter&) {
	return *this;
}

ScalarConverter::~ScalarConverter() {}

void ScalarConverter::convert(const std::string& literal) {
	char c = 0;
	int i = 0;
	float f = 0.0f;
	double d = 0.0;
	Type t = detectType(literal, c, i, f, d);
	if (t == TYPE_INVALID) {
		std::cout << "char: impossible" << std::endl;
		std::cout << "int: impossible" << std::endl;
		std::cout << "float: impossible" << std::endl;
		std::cout << "double: impossible" << std::endl;
		return;
	}
	double base = toDouble(t, c, i, f, d);

	printCharFromDouble(base);
	printIntFromDouble(base);
	printFloatFromDouble(base);
	printDoubleFromDouble(base);
}
