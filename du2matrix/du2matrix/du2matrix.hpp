// du2matrix.hpp
// Krystof Hruby NPRG051 2019/2020

#ifndef MATRIX 
#define MATRIX
#include <vector>
#include <iterator>
template<typename T>
using mat = std::vector<T>;

template<typename T>
class matrix
{
public:
	using value_type = T;
	using reference = T & ;
	using vector_ref = std::vector<T>&;
	using vector_ = std::vector<T>;
	matrix(const matrix<T> & other) 
	{
		row_nbr = other.row_nbr;
		col_nbr = other.col_nbr;
		set_matrix_size();
		copy_from(other.data);
	}

	matrix(matrix<T> && other) : row_nbr(other.row_nbr), col_nbr(other.col_nbr), data(move(other.data)) {}
	
	matrix<T>& operator =(const matrix<T> & other)
	{
		matrix m(other);
		swap(m);
		return *this;
	}

	matrix<T>& operator =(matrix<T> && other)
	{
		swap(other);
		return *this;
	}

	~matrix() {}

	bool operator == (const matrix<T> & other)
	{
		return same_matrix(other);
	}

	bool operator != (const matrix<T> & other)
	{
		return !same_matrix(other);
	}

	matrix(size_t row, size_t col, T val)
	{
		row_nbr = row;
		col_nbr = col;
		set_matrix_size();
		init_matrix(val);
	}

	class cols_t
	{
		matrix<T> *ptr;
	public:
		class column_proxy;
		cols_t(matrix<T> * p) : ptr(p) {}

		using reference = column_proxy;
		struct value_type
		{
			using reference = T & ;
		};

		// represents column of matrix
		class column_proxy
		{
			size_t col;
			matrix * ptr;

			void swap(column_proxy & other)
			{
				std::swap(col, other.col);
				std::swap(ptr, other.ptr);
			}

		public:
			column_proxy(size_t c, matrix*p) : col(c), ptr(p) {}

			column_proxy(const column_proxy & other)
			{
				col = other.col;
				ptr = other.ptr;
			}

			column_proxy(column_proxy && other)
			{
				col = other.col;
				ptr = other.ptr;
			}

			column_proxy & operator = (const column_proxy & other)
			{
				column_proxy tmp(other);
				swap(tmp);
				return *this;
			}

			column_proxy & operator = (column_proxy && other)
			{
				swap(other);
				return *this;
			}

			T& operator[](size_t row)
			{
				return ptr->get_element_at(row, col);
			}

			// iterator that iterates certain column in matrix
			class col_iterator
			{
				matrix * ptr;
				size_t element_nbr;
				size_t col;

				void swap(col_iterator & other)
				{
					std::swap(element_nbr, other.element_nbr);
					std::swap(ptr, other.ptr);
					std::swap(col, other.col);

				}

			public:
				using  iterator_category = std::forward_iterator_tag;
				using pointer = T * ;
				using reference = T & ;
				using value_type = T;
				using difference_type = std::ptrdiff_t;

				col_iterator(matrix * p, size_t el, size_t c) : ptr(p), element_nbr(el), col(c) {}

				col_iterator(const col_iterator & other) : ptr(other.ptr), element_nbr(other.element_nbr), col(other.col) {}

				col_iterator & operator = (const col_iterator & other)
				{
					col_iterator tmp(other);
					swap(tmp);
					return *this;
				}

				col_iterator(col_iterator && other) : ptr(other.ptr), element_nbr(other.element_nbr), col(other.col) {}

				col_iterator & operator = (col_iterator && other)
				{
					swap(other);
					return *this;
				}

				bool operator ==(const col_iterator & ite) const
				{
					return element_nbr == ite.element_nbr && col == ite.col && ptr == ite.ptr;
				}

				bool operator !=(const col_iterator & ite) const
				{
					return element_nbr != ite.element_nbr || col != ite.col || ptr != ite.ptr;
				}

				col_iterator& operator ++()
				{
					++element_nbr;
					return *this;
				}

				col_iterator operator ++(int)
				{
					col_iterator result(*this);
					++(*this);
					return result;
				}

				reference operator *()
				{
					return ptr->get_element_at(element_nbr, col);
				}

				pointer operator ->()
				{
					return &((ptr->get_element_at(element_nbr, col)));
				}
			};

			col_iterator begin()
			{
				return col_iterator(ptr, 0, col);
			}

			col_iterator end()
			{
				return col_iterator(ptr, ptr->row_nbr, col);
			}
		};

		column_proxy operator[](size_t col)
		{
			return column_proxy(col, ptr);
		}

		size_t size()
		{
			return ptr->col_nbr;
		}

		// proxy class to get correct return value from operator -> in cols_iterator
		class arrow_proxy
		{
			column_proxy cp;
		public:
			arrow_proxy(const column_proxy& c) : cp(c) {}

			column_proxy* operator ->()
			{
				return &cp;
			}

		};

		// iterator of columns of matrix
		class cols_iterator
		{
			size_t col;
			matrix * ptr;

			void swap(cols_iterator & other)
			{
				std::swap(col, other.col);
				std::swap(ptr, other.ptr);
			}

		public:
			using iterator_category = std::forward_iterator_tag;
			using pointer = column_proxy * ;
			using reference = column_proxy;
			using value_type = column_proxy;
			using difference_type = std::ptrdiff_t;

			cols_iterator(size_t c, matrix * p) : col(c), ptr(p) {}

			cols_iterator(const cols_iterator & other) : col(other.col), ptr(other.ptr) {}

			cols_iterator(cols_iterator && other) : col(other.col), ptr(move(other.ptr)) {}

			cols_iterator& operator = (const cols_iterator & other)
			{
				cols_iterator tmp(other);
				swap(tmp);
				return *this;
			}

			cols_iterator& operator = (cols_iterator && other)
			{
				swap(other);
				return *this;
			}

			bool operator ==(const cols_iterator & ite) const
			{
				return col == ite.col && ptr == ite.ptr;
			}

			bool operator !=(const cols_iterator & ite) const
			{
				return col != ite.col || ptr != ite.ptr;
			}

			cols_iterator& operator ++()
			{
				++col;
				return *this;
			}

			cols_iterator operator ++(int)
			{
				cols_iterator result(*this);
				++(*this);
				return result;
			}

			column_proxy operator *()
			{
				return column_proxy(col, ptr);
			}

			arrow_proxy operator ->()
			{
				return arrow_proxy{ column_proxy{col, ptr} };
			}
		};

		cols_iterator begin()
		{
			return cols_iterator(0, ptr);
		}

		cols_iterator end()
		{
			return cols_iterator(ptr->col_nbr, ptr);
		}

	};

	class rows_t
	{
		matrix<T> *ptr;
	public:
		class row_proxy;
		rows_t(matrix<T> * p) : ptr(p) {}

		using reference = row_proxy;
		struct value_type
		{
			using reference = T & ;
		};

		// represents row of matrix
		class row_proxy
		{
			size_t row;
			matrix * ptr;

			void swap(row_proxy & other)
			{
				std::swap(row, other.row);
				std::swap(ptr, other.ptr);
			}

		public:
			row_proxy(size_t r, matrix*p) : row(r), ptr(p) {}

			row_proxy(const row_proxy & other) : row(other.row), ptr(other.ptr) {}

			row_proxy(row_proxy && other) : row(other.row), ptr(other.ptr) {}

			row_proxy & operator = (const row_proxy & other)
			{
				row_proxy tmp(other);
				swap(tmp);
				return *this;
			}

			row_proxy & operator = (row_proxy && other)
			{
				swap(other);
				return *this;
			}

			T& operator[](size_t col)
			{
				return ptr->get_element_at(row, col);
			}

			// iterates over certain row
			class row_iterator
			{
				matrix * ptr;
				size_t element_nbr;
				size_t row;

				void swap(row_iterator & other)
				{
					std::swap(element_nbr, other.element_nbr);
					std::swap(ptr, other.ptr);
					std::swap(row, other.row);
				}

			public:
				using iterator_category = std::forward_iterator_tag;
				using pointer = T * ;
				using reference = T & ;
				using value_type = T;
				using difference_type = std::ptrdiff_t;

				row_iterator(matrix * p, size_t el, size_t r) : ptr(p), element_nbr(el), row(r) {}

				row_iterator(const row_iterator & other) : ptr(other.ptr), element_nbr(other.element_nbr), row(other.row) {}

				row_iterator & operator = (const row_iterator & other)
				{
					row_iterator tmp(other);
					swap(tmp);
					return *this;
				}

				row_iterator(row_iterator && other) : ptr(other.ptr), element_nbr(other.element_nbr), row(other.row) {}

				row_iterator & operator = (row_iterator && other)
				{
					swap(other);
					return *this;
				}

				bool operator ==(const row_iterator & ite) const
				{
					return element_nbr == ite.element_nbr && row == ite.row && ptr == ite.ptr;
				}

				bool operator !=(const row_iterator & ite) const
				{
					return element_nbr != ite.element_nbr || row != ite.row || ptr != ite.ptr;
				}

				row_iterator& operator ++()
				{
					++element_nbr;
					return *this;
				}

				row_iterator operator ++(int)
				{
					row_iterator result(*this);
					++(*this);
					return result;
				}

				reference operator *()
				{
					return ptr->get_element_at(row, element_nbr);
				}

				pointer operator ->()
				{
					return &(ptr->get_element_at(row, element_nbr));
				}
			};

			row_iterator begin()
			{
				return row_iterator(ptr, 0, row);
			}

			row_iterator end()
			{
				return row_iterator(ptr, ptr->col_nbr, row);
			}
		};

		row_proxy operator[](size_t row)
		{
			return row_proxy(row, ptr);
		}

		size_t size()
		{
			return ptr->row_nbr;
		}

		// proxy class to asure correct return value of operator -> of rows_iterator
		class arrow_proxy
		{
			row_proxy rp;
		public:
			arrow_proxy(const row_proxy& r) : rp(r) {}

			row_proxy* operator ->()
			{
				return &rp;
			}

		};

		// iterates over the rows of matrix
		class rows_iterator
		{
			size_t row;
			matrix * ptr;

			void swap(rows_iterator & other)
			{
				std::swap(row, other.row);
				std::swap(ptr, other.ptr);
			}

		public:
			using iterator_category = std::forward_iterator_tag;
			using pointer = row_proxy * ;
			using reference = row_proxy;
			using value_type = row_proxy;
			using difference_type = std::ptrdiff_t;

			rows_iterator(size_t r, matrix * p) : row(r), ptr(p) {}

			rows_iterator(const rows_iterator & other) : row(other.row), ptr(other.ptr) {}

			rows_iterator(rows_iterator && other) : row(other.row), ptr(move(other.ptr)) {}

			rows_iterator& operator = (const rows_iterator & other)
			{
				rows_iterator tmp(other);
				swap(tmp);
				return *this;
			}

			rows_iterator& operator = (rows_iterator && other)
			{
				swap(other);
				return *this;
			}

			bool operator ==(const rows_iterator & ite) const
			{
				return row == ite.row && ptr == ite.ptr;
			}

			bool operator !=(const rows_iterator & ite) const
			{
				return row != ite.row || ptr != ite.ptr;
			}

			rows_iterator& operator ++()
			{
				++row;
				return *this;
			}

			rows_iterator operator ++(int)
			{
				rows_iterator result(*this);
				++(*this);
				return result;
			}

			row_proxy operator *()
			{
				return row_proxy(row, ptr);
			}

			arrow_proxy operator ->()
			{
				return arrow_proxy{ row_proxy{row, ptr} };
			}
		};

		rows_iterator begin()
		{
			return rows_iterator(0, ptr);
		}

		rows_iterator end()
		{
			return rows_iterator(ptr->row_nbr, ptr);
		}
	};

	cols_t cols()
	{
		return cols_t(this);
	}

	rows_t rows()
	{
		return rows_t(this);
	}

	size_t row_number() const
	{
		return row_nbr;
	}

	// proxy row of matrix. Used to implement [][]
	class row_proxy_index
	{
		size_t row;
		matrix * ptr;

		void swap(row_proxy_index & other)
		{
			std::swap(row, other.row);
			std::swap(ptr, other.ptr);
		}

	public:
		row_proxy_index(size_t r, matrix*p) : row(r), ptr(p) {}

		row_proxy_index(const row_proxy_index & other) : row(other.row), ptr(other.ptr) {}

		row_proxy_index(row_proxy_index && other) : row(other.row), ptr(other.ptr) {}

		row_proxy_index & operator = (const row_proxy_index & other)
		{
			row_proxy_index tmp(other);
			swap(tmp);
			return *this;
		}

		row_proxy_index & operator = (row_proxy_index && other)
		{
			swap(other);
			return *this;
		}

		T& operator[](size_t col)
		{
			return ptr->get_element_at(row, col);
		}
	};

	size_t col_number() const
	{
		return col_nbr;
	}

	row_proxy_index operator [](size_t row)
	{
		return row_proxy_index(row, this);
	}
private:
	std::vector<T> data;
	size_t row_nbr, col_nbr;

	bool same_matrix(const matrix<T> & other)
	{
		if (row_nbr != other.row_nbr)
			return false;

		if (col_nbr != other.col_nbr)
			return false;

		for (size_t i = 0; i < row_nbr * col_nbr; ++i)
		{
			if (data[i] != other.data[i])
				return false;
		}
		return true;
	}

	reference get_element_at(size_t row, size_t col)
	{
		return data[row * col_nbr + col];
	}

	void swap(matrix<T> & other)
	{
		std::swap(data, other.data);
		std::swap(row_nbr, other.row_nbr);
		std::swap(col_nbr, other.col_nbr);
	}

	void set_matrix_size()
	{
		data.resize(row_nbr * col_nbr);
	}

	void init_matrix(T val)
	{
		for (size_t i = 0; i < row_nbr * col_nbr; ++i)
		{
			data[i] = val;
		}
	}

	void copy_from(const mat<T> & other)
	{
		for (size_t i = 0; i < row_nbr * col_nbr; ++i)
		{
			data[i] = other[i];
		}
	}
};
#endif // MATRIX
