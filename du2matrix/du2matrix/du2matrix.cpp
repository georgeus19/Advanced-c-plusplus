﻿#include "pch.h"
#include "du2matrix.hpp"

#include <iostream>
#include <algorithm>

using my_matrix = matrix< int>;

int cnt = 100;

void f1(my_matrix::cols_t::value_type::reference x)
{
	x = ++cnt;
}

void f2(my_matrix::cols_t::reference r)
{
	std::for_each(r.begin(), r.end(), f1);
}

void f3(my_matrix::rows_t::value_type::reference x)
{
	std::cout << x << " ";
}

void f4(my_matrix::rows_t::reference r)
{
	std::for_each(r.begin(), r.end(), f3);
	std::cout << std::endl;
}

#include <string>
using std::string;
struct Complex
{
	double re = 0;
	double im = 0;
};

int main(int, char * *)
{
	try {



		std::cout << "string as T test, columns" << std::endl;
		matrix<string> s(3, 4, string());


		for (auto&& col : s.cols())
		{
			for (auto && el : col)
			{
				el = "OMG";
			}
		}
		s.cols()[3][2] = "2_2";
		s[0][1] = "2_2";
		for (auto&& col : s.cols())
		{
			for (auto && el : col)
			{
				std::cout << el << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "string as T test, rows" << std::endl;
		for (auto&& row : s.rows())
		{
			for (auto && el : row)
			{
				el = "RADKY";
			}
		}

		for (auto&& row : s.rows())
		{
			for (auto && el : row)
			{
				std::cout << el << " ";
			}
			std::cout << std::endl;
		}


		my_matrix a(3, 4, 0);  // matice 3 radky * 4 sloupce inicializovana nulami
		matrix b{ a };
		matrix c(10, 20, 1);
		c = a;

		std::for_each(b.cols().begin(), b.cols().end(), f2);

		c = b;
		if (c == b)
			std::cout << "overload oper == -> true;\n";
		std::cout << c[0][2] << std::endl;
		std::cout << c.rows()[0][2] << std::endl;
		c[0][2] = b[1][1];
		if (c != b)
			std::cout << "not equal matrices\n";
		std::cout << c[0][2] << std::endl;
		std::cout << c.rows()[0][2] << std::endl;
		std::cout << "Pozmenena matice c: (2xfor cyklus:)" << std::endl;
		for (size_t i = 0; i < c.row_number(); i++)
		{
			for (size_t j = 0; j < c.col_number(); j++)
			{
				std::cout << c[i][j] << " ";
			}
			std::cout << std::endl;
		}

		std::cout << std::endl;
		std::for_each(c.rows().begin(), c.rows().end(), f4);

		std::cout << std::endl;
		for (auto&& r : c.rows())
			f4(r);

		std::cout << std::endl;
		// postfix ++
		auto it1 = c.cols()[0].begin();
		auto it0 = it1;
		auto it2 = it1++; //i it2 je zvyseny??
		*it2 = -99;

		int tmp = 5;
		auto tmp1 = tmp++;

		// iterators
		std::for_each(c.rows().begin(), c.rows().end(),
			[](my_matrix::rows_t::reference row)
		{
			std::for_each(row.begin(), row.end(),
				[](my_matrix::rows_t::value_type::reference el) {
				std::cout << el << ' ';
			});
			std::cout << '\n';
		});

		// prefix ++, ->
		matrix cc(6, 5, Complex{});
		auto it3 = cc.rows().begin();
		auto it4 = it3->begin();
		(++it4)->re = 4;
		(++it3)->begin()->im = 7;

		auto it5 = cc.cols().begin();
		auto it6 = it5->begin();
		(++it6)->re = 4; 
		(++it5)->begin()->im = 7; //segment error!
		auto col = cc.cols().begin();
		auto it = col->begin();
		for (size_t i = 0; i < 4; i++)
		{
			it->re = 5;
			++it;
		}
		for (auto&& row : cc.rows())
		{
			for (auto&& el : row)
				std::cout << el.re << "+" << el.im << "i ";
			std::cout << '\n';
		}


		// range-base for
		for (auto&& col : cc.cols())
		{
			for (auto&& el : col)
				std::cout << el.re << "+" << el.im << "i ";
			std::cout << '\n';
		}
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}
