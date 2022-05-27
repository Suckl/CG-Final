#include<fstream>
#include<iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include<cmath>
#include <sstream>
#include <cassert>
#include <cctype>
#include <utility>

namespace microobj {

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) \
  (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>(10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

// 有序储存了所有的顶点与法线
typedef struct {
  std::vector<float> vertices;   
  std::vector<float> normals;    
  std::vector<float> texcoords;
} attrib_t;

// 有序储存了这个面所引用的顶点与法线下标
typedef struct {
  int vertex_index;
  int normal_index;
  int texcoord_index;
} index_t;

// 有序储存了所有的面与对应的下标关系
typedef std::vector<index_t> face_indices_t;

struct vertex_index {
  int v_idx, vt_idx, vn_idx;
  vertex_index() : v_idx(-1), vt_idx(-1), vn_idx(-1) {}
  explicit vertex_index(int idx) : v_idx(idx), vt_idx(idx), vn_idx(idx) {}
  vertex_index(int vidx, int vtidx, int vnidx)
      : v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};

static std::istream &safeGetline(std::istream &is, std::string &t) {
  t.clear();

  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.

  std::istream::sentry se(is, true);
  std::streambuf *sb = is.rdbuf();

  for (;;) {
    int c = sb->sbumpc();
    switch (c) {
      case '\n':
        return is;
      case '\r':
        if (sb->sgetc() == '\n') sb->sbumpc();
        return is;
      case EOF:
        // Also handle the case when the last line has no line ending
        if (t.empty()) is.setstate(std::ios::eofbit);
        return is;
      default:
        t += static_cast<char>(c);
    }
  }
}

static bool tryParseDouble(const char *s, const char *s_end, double *result) {
  if (s >= s_end) {
    return false;
  }

  double mantissa = 0.0;
  // This exponent is base 2 rather than 10.
  // However the exponent we parse is supposed to be one of ten,
  // thus we must take care to convert the exponent/and or the
  // mantissa to a * 2^E, where a is the mantissa and E is the
  // exponent.
  // To get the final double we will use ldexp, it requires the
  // exponent to be in base 2.
  int exponent = 0;

  // NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
  // TO JUMP OVER DEFINITIONS.
  char sign = '+';
  char exp_sign = '+';
  char const *curr = s;

  // How many characters were read in a loop.
  int read = 0;
  // Tells whether a loop terminated due to reaching s_end.
  bool end_not_reached = false;

  /*
          BEGIN PARSING.
  */

  // Find out what sign we've got.
  if (*curr == '+' || *curr == '-') {
    sign = *curr;
    curr++;
  } else if (IS_DIGIT(*curr)) { /* Pass through. */
  } else {
    goto fail;
  }

  // Read the integer part.
  end_not_reached = (curr != s_end);
  while (end_not_reached && IS_DIGIT(*curr)) {
    mantissa *= 10;
    mantissa += static_cast<int>(*curr - 0x30);
    curr++;
    read++;
    end_not_reached = (curr != s_end);
  }

  // We must make sure we actually got something.
  if (read == 0) goto fail;
  // We allow numbers of form "#", "###" etc.
  if (!end_not_reached) goto assemble;

  // Read the decimal part.
  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      static const double pow_lut[] = {
          1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
      };
      const int lut_entries = sizeof pow_lut / sizeof pow_lut[0];

      // NOTE: Don't use powf here, it will absolutely murder precision.
      mantissa += static_cast<int>(*curr - 0x30) *
                  (read < lut_entries ? pow_lut[read] : std::pow(10.0, -read));
      read++;
      curr++;
      end_not_reached = (curr != s_end);
    }
  } else if (*curr == 'e' || *curr == 'E') {
  } else {
    goto assemble;
  }

  if (!end_not_reached) goto assemble;

  // Read the exponent part.
  if (*curr == 'e' || *curr == 'E') {
    curr++;
    // Figure out if a sign is present and if it is.
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '-')) {
      exp_sign = *curr;
      curr++;
    } else if (IS_DIGIT(*curr)) { /* Pass through. */
    } else {
      // Empty E is not allowed.
      goto fail;
    }

    read = 0;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      exponent *= 10;
      exponent += static_cast<int>(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    exponent *= (exp_sign == '+' ? 1 : -1);
    if (read == 0) goto fail;
  }

assemble:
  *result =
      (sign == '+' ? 1 : -1) *
      (exponent ? std::ldexp(mantissa * std::pow(5.0, exponent), exponent) : mantissa);
  return true;
fail:
  return false;
}

static inline float parseReal(const char **token, double default_value = 0.0) {
  (*token) += strspn((*token), " \t");
  const char *end = (*token) + strcspn((*token), " \t\r");
  double val = default_value;
  tryParseDouble((*token), end, &val);
  float f = static_cast<float>(val);
  (*token) = end;
  return f;
}

static inline void parseReal2(float *x, float *y, const char **token,
                               const double default_x = 0.0,
                               const double default_y = 0.0) {
  (*x) = parseReal(token, default_x);
  (*y) = parseReal(token, default_y);
}


static inline void parseReal3(float *x, float *y, float *z, const char **token,
                               const double default_x = 0.0,
                               const double default_y = 0.0,
                               const double default_z = 0.0) {
  (*x) = parseReal(token, default_x);
  (*y) = parseReal(token, default_y);
  (*z) = parseReal(token, default_z);
}

static inline int fixIndex(int idx, int n) {
  if (idx > 0) return idx - 1;
  if (idx == 0) return 0;
  return n + idx;  // negative value = relative
}

static vertex_index parseTriple(const char **token, int vsize, int vnsize,
                                int vtsize) {
    vertex_index vi(-1);
    // 第一种情况，只有一个顶点
    vi.v_idx = fixIndex(atoi((*token)), vsize);
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return vi;
    }
    (*token)++;
    // 第二种情况，顶点+法线坐标
    // i//k
    if ((*token)[0] == '/') {
        (*token)++;
        vi.vn_idx = fixIndex(atoi((*token)), vnsize);
        (*token) += strcspn((*token), "/ \t\r");
        return vi;
    }
    // 存在纹理坐标，但是我们不对纹理坐标作任何处理
    // i/j/k or i/j
    vi.vt_idx = fixIndex(atoi((*token)), vtsize);
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return vi;
    }
    // 如果还有法线坐标，记录法线坐标
    // i/j/k
    (*token)++;  // skip '/'
    vi.vn_idx = fixIndex(atoi((*token)), vnsize);
    (*token) += strcspn((*token), "/ \t\r");
    return vi;
}

bool MicroLoadObj(attrib_t * attrib, face_indices_t * face_indices,const char *filename){
    std::vector<float> v;
    std::vector<float> vn;
    std::vector<float> vt;
    std::vector<index_t> face;
    attrib->vertices.clear();
    attrib->normals.clear();
    face_indices->clear();
    std::ifstream ifs(filename);
    if (!ifs) return false;
    std::string linebuf;
    while (ifs.peek() != -1) {
        safeGetline(ifs, linebuf);
        // 清洗
        if (linebuf.size() > 0) 
        {
            if (linebuf[linebuf.size() - 1] == '\n')
                linebuf.erase(linebuf.size() - 1);
        }
        if (linebuf.size() > 0) 
        {
            if (linebuf[linebuf.size() - 1] == '\r')
                linebuf.erase(linebuf.size() - 1);
        }
        // 跳过空白行
        if (linebuf.empty()) 
        {
            continue;
        }
        // 跳过先导空格
        const char *token = linebuf.c_str();
        token += strspn(token, " \t");
        if (token[0] == '\0') continue;  // 空白行
        if (token[0] == '#') continue;  // 注释行
        // 顶点处理
        if (token[0] == 'v' && IS_SPACE((token[1]))) 
        {
            token += 2;
            float x, y, z;
            parseReal3(&x, &y, &z, &token);
            v.push_back(x);
            v.push_back(y);
            v.push_back(z);
            continue;
        }
        // 法线处理
        if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) 
        {
            token += 3;
            float x, y, z;
            parseReal3(&x, &y, &z, &token);
            vn.push_back(x);
            vn.push_back(y);
            vn.push_back(z);
            continue;
        }
        // 纹理坐标
        if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
            token += 3;
            float x, y;
            parseReal2(&x, &y, &token);
            vt.push_back(x);
            vt.push_back(y);
            continue;
        }
        // 面处理
        if (token[0] == 'f' && IS_SPACE((token[1]))) 
        {
            std::vector<vertex_index> sface;
            sface.reserve(3);
            token += 2;
            token += strspn(token, " \t");
            while (!IS_NEW_LINE(token[0])) {
                vertex_index vi = parseTriple(&token, static_cast<int>(v.size() / 3),
                                            static_cast<int>(vn.size() / 3),
                                            static_cast<int>(vt.size() / 2));
                sface.push_back(vi);
                size_t n = strspn(token, " \t\r");
                token += n;
            }
            // 将多边形面处理成三角形面
            vertex_index i0 = sface[0];
            vertex_index i1(-1);
            vertex_index i2 = sface[1];
            size_t npolys = sface.size();
            for (size_t k = 2; k < npolys; k++) {
              i1 = i2;
              i2 = sface[k];
              // 递归得到三个下标
              index_t idx0, idx1, idx2;
              idx0.vertex_index = i0.v_idx;
              idx0.normal_index = i0.vn_idx;
              idx0.texcoord_index = i0.vt_idx;
              idx1.vertex_index = i1.v_idx;
              idx1.normal_index = i1.vn_idx;
              idx1.texcoord_index = i1.vt_idx;
              idx2.vertex_index = i2.v_idx;
              idx2.normal_index = i2.vn_idx;
              idx2.texcoord_index = i2.vt_idx;
              face.push_back(idx0);
              face.push_back(idx1);
              face.push_back(idx2);
            }
            continue;
        }
    }
    attrib->normals.swap(vn);
    attrib->vertices.swap(v);
    attrib->texcoords.swap(vt);
    face_indices->swap(face);
    return true;
}
}