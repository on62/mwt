// linear_algebra.cpp

#include "linear_algebra.h"

#include <algorithm>
#include <fstream>

using namespace std;

Matrix::Matrix(size_t d) : d{d} { 
  if (d == 0) throw std::logic_error{"no 0-d matrices!"};
}

size_t get_d(const std::initializer_list<Vector> &il) {
  if (il.size()) {
    return begin(il)->size();
  } else {
    throw std::logic_error{"get_d: empty init-list"};
  }
}

Matrix::Matrix(std::initializer_list<Vector> &&il) 
    : d{get_d(il)}, vectors{move(il)} {
  if (!check()) throw std::logic_error{"Matrix: bad init-list"};
}

bool Matrix::check() const {
  return all_of(vectors.begin(), vectors.end(), 
      [this](const Vector &v) { return v.size() == d; });
}

Matrix::Matrix(const Matrix &m) : d{m.d}, vectors{m.vectors} {}

Matrix::Matrix(Matrix &&m) : d{m.d}, vectors{move(m.vectors)} {}

Matrix &Matrix::operator=(const Matrix &m) {
  if (d != m.d) {
    throw std::logic_error{"Matrix copy assignment: unmatched d"};
  }
  vectors = m.vectors;
  return *this;
}

Matrix &Matrix::operator=(Matrix &&m) {
  if (d != m.d) {
    throw std::logic_error{"Matrix move assignment: unmatched d"};
  }
  vectors = move(m.vectors);
  return *this;
}

Matrix &Matrix::operator=(std::initializer_list<Vector> &&il) {
  vectors = move(Vectors{il});
  return *this;
}

void Matrix::push_back(const Vector &v) { 
  if (v.size() != d) {
    string ineq = to_string(v.size()) + " != " + to_string(d);
    throw std::out_of_range{"push_back(copy), " + ineq};
  }
  vectors.push_back(v); 
}

void Matrix::push_back(Vector &&v) { 
  if (v.size() != d) {
    string ineq = to_string(v.size()) + " != " + to_string(d);
    throw std::out_of_range{"push_back(move), " + ineq};
  }
  vectors.push_back(move(v)); 
}

// read in Vector
// Vector must be correct size before reading
istream& operator>>(istream& i, Vector &v) {
  for (auto &v_i : v) {
    i >> v_i;
  }
  if (i.fail()) {
    throw input_error{"failed to read vector: istream failed"};
  }
  return i;
}

// read in Matrix
// Matrix must have d set before reading (enforced by constructor)
istream& operator>>(istream& i, Matrix &M) {
  try{
    do {
      i >> M.add_Vector() >> ws;
    } while (i.good());
  } catch (input_error e) {
    cerr << "error reading matrix, vector " << M.size() << endl;
    throw;
  }
  if (i.fail()) {
    throw input_error{"failed to read matrix: istream failed"};
  }
  return i;
}

// read in VPoly
// vpoly must have d set before reading (enforced by constructor)
istream& operator>>(istream& i, VPoly &vpoly) {
  // determines which Matrix gets the next vector
  char who{0};
  try {
    do {
      // check for U/V, get next vector and ws
      if (who == 'V' || ( i.peek() == 'V' && (who = i.get()) )) {
        i >> vpoly.V.add_Vector() >> ws;
      } else if (who == 'U' || ( i.peek() == 'U' && (who = i.get()) )) {
        i >> vpoly.U.add_Vector() >> ws;
      } else {
        throw input_error{"neither U (rays) nor V (points) chosen"};
      }
    } while (i.good());
  } catch (input_error e) {
    cerr << "error reading matrix, vector " 
         << vpoly.U.size() + vpoly.V.size() << endl;
    throw;
  }
  if (i.fail()) {
    throw input_error{"failed to read vpoly: istream failed"};
  }
  return i;
}

// output Vector
ostream& operator<<(ostream& o, const Vector& v) {
  for (auto &&val : v) { o << val << " "; }
  return o;
}

// output Matrix
ostream& operator<<(ostream& o, const Matrix& M) {
  o << M.d << endl;
  for (auto &&v : M) { o << v << endl; }
  return o;
}

// output VPoly
ostream& operator<<(ostream& o, const VPoly &vpoly) {
  o << vpoly.d << endl;
  if (!vpoly.U.empty()) {
    o << "U" << endl;
    for (auto &&u : vpoly.U) { o << u << endl; }
  }
  if (!vpoly.V.empty()) {
    o << "V" << endl;
    for (auto &&v : vpoly.V) { o << v << endl; }
  }
  return o;
}

size_t read_dimension(std::istream &i) {
  int d;
  i >> d >> ws;
  if (d <= 0) {
    throw input_error{"bad d: "s + to_string(d)};
  }
  return static_cast<size_t>(d);
}

size_t get_d(const std::initializer_list<Vector> &Uil,
             const std::initializer_list<Vector> &Vil) {
  if (Uil.size()) { 
    return begin(Uil)->size(); 
  } else if (Vil.size()) {
    return begin(Vil)->size(); 
  } else {
    return 0;
  }
}

VPoly::VPoly(std::initializer_list<Vector>&& Uil, 
             std::initializer_list<Vector>&& Vil) 
    : d{get_d(Uil,Vil)}, U{d}, V{d} {
  U = move(Uil);
  V = move(Vil);
  if (!check()) throw std::logic_error{"VPoly: bad init-list"};
}

bool VPoly::check() const {
  return all_of(U.begin(), U.end(), 
          [this](const Vector &v) { return v.size() == d; }) &&
         all_of(U.begin(), U.end(), 
          [this](const Vector &v) { return v.size() == d; });
}

// factory needed because const member must be set for operator>> to work
Matrix Matrix::read_Matrix(std::istream& i) {
  Matrix result{read_dimension(i)};
  i >> result;
  return result;
}

Vectors::iterator       Matrix::begin() { return vectors.begin(); }
Vectors::iterator       Matrix::end()   { return vectors.end(); }
Vectors::const_iterator Matrix::begin() const { return vectors.begin(); }
Vectors::const_iterator Matrix::end()   const { return vectors.end(); }

bool   Matrix::empty() const { return vectors.empty(); }
size_t Matrix::size()  const { return vectors.size(); }
Vector &Matrix::back()       { return vectors.back(); }

Vector &Matrix::add_Vector() {
  vectors.emplace_back(d);
  return vectors.back();
}

// factory needed because const member must be set for operator>> to work
VPoly VPoly::read_VPoly(std::istream& i) {
  VPoly result{read_dimension(i)};
  i >> result;
  return result;
}

input_error::input_error(const char*s) : std::runtime_error{s} {}
input_error::input_error(const std::string &s) : std::runtime_error{s} {}

// If I had to write that one more time I'd templatize it...

int usage() {
  cerr << ifstream{"../usage.txt"}.rdbuf();
  return 0;
}

// Vector utilities

// basis vectors
Vector e_k(size_t d, size_t k) {
  Vector result(d);
  result[k] = 1;
  return result;
}

// concatentation
Vector concatenate(const Vector &l, const Vector &r) {
  Vector result(l.size() + r.size());
  copy(begin(l), end(l), begin(result));
  copy(begin(r), end(r), next(begin(result), l.size()));
  return result;
}

// Matrix utilities

// column selection
Vector get_column(const Matrix &M, size_t k) {
  if (!(0 <= k && k < M.d)) {
    throw std::out_of_range("k < 0 || M.d <= k");
  }
  Vector result(M.size());
  size_t result_row{0};
  for (auto &&row : M) {
    result[result_row++] = row[k];
  }
  return result;
}

// Matrix transpose
Matrix transpose(const Matrix &M) {
  if (M.empty()) {
    return M;
  }
  Matrix result{M.size()};
  // for every column of M,
  for (size_t k = 0; k < M.d; ++k) {
    result.push_back(get_column(M,k));
  }
  return result;
}

// slice each vector in M with s
Matrix slice_matrix(const Matrix &M, const std::slice &s) {
  Matrix result{s.size()};
  transform(M.begin(), M.end(), back_inserter(result),
    [s](const Vector &v) { return v[s]; });
  return result;
}
