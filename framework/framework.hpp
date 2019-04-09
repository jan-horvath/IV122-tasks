#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <cassert>

namespace svg {

    const std::string FILE_NAME = "output_image.svg";
    const double IMAGE_HEIGHT = 1080;
    const double IMAGE_WIDTH = 1920;

    const std::vector<std::string> COLORS {
        "black", "red", "yellow", "green", "cyan", "blue", "pink",
        "tomato", "greenyelow", "turquoise", "dodgerblue", "purple", "mediumvioletred",
        "darkorange", "lightgreen", "aquamarine", "royalblue", "mediumpurple", "hotpink"
    };

    struct Matrix {
        std::vector<double> _d;
        size_t _rows;
        size_t _cols;

        Matrix(size_t rows, size_t cols) : _d(rows*cols, 0), _rows(rows), _cols(cols) {}

        Matrix(std::initializer_list<double> list, size_t rows, size_t cols) : _d(list), _rows(rows), _cols(cols) {}

        Matrix operator* (const Matrix& mat) {
            assert(_cols == mat._rows);

            Matrix product(_rows, mat._cols);
            for (size_t row = 0; row < _rows; ++row) {
                for (size_t col = 0; col < mat._cols; ++col) {

                    for (size_t i = 0; i < _cols; ++i) {
                        product._d.at(row*mat._cols + col) += _d.at(_cols*row+i) * mat._d.at(i*mat._cols+col);
                    }

                }
            }
            return std::move(product);
        }

        //TODO add operator *=
    };

    //TODO translation, scaling, rotation, replexion(axis)
    //TODO transformation composition, draw(geom. object), applyToLine(line, tranformation), applyToObject(object = vector<lines>, transformation)
    //TODO generateSquare, generateTraingle, generate[someOtherObject]

    Matrix translation(double X, double Y) {

    }

    struct Vector {
        double X;
        double Y;

        bool operator==(const Vector &rhs) const {
            return (fabs(X - rhs.X) < 0.001) && (fabs(Y - rhs.Y) < 0.001);
        }

        bool operator!=(const Vector &rhs) const {
            return !(rhs == *this);
        }

        Vector operator*(double n) {
            return {this->X * n, this->Y * n};
        }

        Vector norm() const {
            double length = sqrt(X*X + Y*Y);
            return {X/length, Y/length};
        }

        double getLength() const {
            return sqrt(X*X + Y*Y);
        }

        double getAngle(const Vector& v) const {
            double epsilon = 0.9999;
            return acos((X*v.X + Y*v.Y)/(this->getLength() * v.getLength()) * epsilon);
        }
    };


    struct Point {
        double X = 0.0;
        double Y = 0.0;

        bool operator==(const Point &rhs) const {
            return X == rhs.X &&
                   Y == rhs.Y;
        }

        bool operator!=(const Point &rhs) const {
            return !(rhs == *this);
        }

        Vector operator-(const Point& p) const {
            return {this->X - p.X, this->Y - p.Y};
        }

        Point operator+(const Vector& v) const {
            return {this->X + v.X, this->Y + v.Y};
        }
    };

    struct LineSegment {
        constexpr static double NO_INTERSECTION = std::numeric_limits<double>::min();
        Point P1;
        Point P2;

        Vector getVec() const {
            return P2 - P1;
        }

        Point intersect(const LineSegment& ls) const {
            if ((ls.getVec().norm() == getVec().norm()) || (ls.getVec().norm()*(-1) == getVec().norm())) {
                return {NO_INTERSECTION, NO_INTERSECTION};
            }

            Point P;
            double EPSILON = 0.01;
            P.X = ((this->P1.X*this->P2.Y - this->P1.Y*this->P2.X) * (ls.P1.X - ls.P2.X)
                   - (this->P1.X - this->P2.X) * (ls.P1.X*ls.P2.Y - ls.P1.Y*ls.P2.X)) /
                   ((this->P1.X - this->P2.X)*(ls.P1.Y - ls.P2.Y)
                   - (this->P1.Y - this->P2.Y)*(ls.P1.X - ls.P2.X));
            P.Y = ((this->P1.X*this->P2.Y - this->P1.Y*this->P2.X) * (ls.P1.Y - ls.P2.Y)
                   - (this->P1.Y - this->P2.Y) * (ls.P1.X*ls.P2.Y - ls.P1.Y*ls.P2.X)) /
                  ((this->P1.X - this->P2.X)*(ls.P1.Y - ls.P2.Y)
                   - (this->P1.Y - this->P2.Y)*(ls.P1.X - ls.P2.X));
            double lambda1 = (P.X - ls.P1.X)/(ls.P2.X - ls.P1.X);
            double lambda2 = (P.X - this->P1.X)/(this->P2.X - this->P1.X);
            if (__isnan(lambda1)) {lambda1 = (P.Y - ls.P1.Y)/(ls.P2.Y - ls.P1.Y);}
            if (__isnan(lambda2)) {lambda2 = (P.Y - this->P1.Y)/(this->P2.Y - this->P1.Y);}

            if ((lambda1 < 0 + EPSILON) || (lambda1 > 1 - EPSILON) || (lambda2 < 0 + EPSILON) || (lambda2 > 1 - EPSILON)) {
                return {NO_INTERSECTION, NO_INTERSECTION};
            }

            return P;
        }

        double getLength() const {
            Vector vec = this->getVec();
            return sqrt(vec.X*vec.X + vec.Y*vec.Y);
        }
    };

    class SVGFile {
    public:
        SVGFile() : SVGFile(FILE_NAME) {}

        SVGFile(const std::string &file_name) : SVGFile(file_name, IMAGE_HEIGHT, IMAGE_WIDTH) {}

        SVGFile(const std::string &file_name, double height, double width, const std::string &color = "white") :
        m_file(file_name), m_height(height), m_width(width) {
            m_file << "<html>\n<body>\n\n"
                   << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink= \"http://www.w3.org/1999/xlink\" "
                   << "viewBox=\"0 0 " << width << " " << height << "\">\n"
                   << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
        }

        /**
         *
         * @param a - Start point
         * @param b - End point
         * @param col - Color
         * @param upscale - True means, that point coordinates are in interval <-1, 1> and need to be upscaled for SVG
         */
        void addLine(Point a, Point b, const std::string& col = COLORS[0], bool upscale = false) {
            if (upscale) {
                a.X = m_width / 2 + a.X * m_width / 2;
                b.X = m_width / 2 + b.X * m_width / 2;
                a.Y = m_height / 2 + a.Y * m_height / 2;
                b.Y = m_height / 2 + b.Y * m_height / 2;

            }
            m_file << "   <line x1=\"" << a.X << "\" y1=\"" << a.Y
                   << "\" x2=\"" << b.X << "\" y2=\"" << b.Y
                   << "\" stroke=\"" << col << "\" />" << std::endl;
        }

        /**
         *
         * @param c - center
         * @param rad - radius
         * @param fill - whether the circle should be filled with colour
         * @param col - Colour for circle and fill
         * @param upscale - True means, that point coordinates are in interval <-1, 1>, radius is in interval <0, 2>
         * and need to be upscaled for SVG
         */
        void addCircle(Point c, double rad, bool fill, const std::string& col = COLORS[0], bool upscale = false) {
            if (upscale) {
                c.X = m_width / 2 + c.X * m_width / 2;
                c.Y = m_height / 2 + c.Y * m_height / 2;
            }

            m_file << "   <circle cx=\"" << c.X << "\" cy=\"" << c.Y
                   << "\" r=\"" << rad
                   << "\" stroke=\"" << col << "\" ";
            if (fill) {
                m_file << "fill=\"" << col << "\" ";
            }
            m_file << "/>" << std::endl;
        }

        /**
         * Creates a filled rectangle
         * @param c - center
         * @param width - width of rectangle
         * @param height - height of rectangle
         * @param upscale - True means, that point coordinates are in interval <-1, 1>, width and height are
         * in interval <0, 2> and need to be upscaled for SVG
         */
        void addRect(Point c, double width, double height, const std::string& col = COLORS[0], bool upscale = false) {
            if (upscale) {
                width *= m_width / 2;
                height *= m_height / 2;
                c.X = m_width / 2 + c.X * m_width / 2 - width / 2;
                c.Y = m_height / 2 + c.Y * m_height /2 - height / 2;
            } else {
                c.X -= width / 2;
                c.Y -= height / 2;
            }

            m_file  << "   <rect x=\"" << c.X << "\" y=\"" << c.Y
                    << "\" width=\"" << width
                    << "\" height=\"" << height
                    << "\" fill=\"" << col
                    << "\" />" << std::endl;
        }

        ~SVGFile() {
            m_file << "</svg>\n\n</body>\n</html>";
        }

        const double m_height;
        const double m_width;

    private:
        std::ofstream m_file;
    };

    class Turtle {
    public:
        Turtle(const std::string &filename) :
                m_file(filename),
                m_pos{m_file.m_width/2, m_file.m_height/2} {}

        Turtle(const std::string &filename, double height, double width) :
                m_file(filename, height, width),
                m_pos{width/2, height/2} {}

        Turtle(Turtle &) = delete;
        void operator=(const Turtle&) = delete;

        void forward(double len) {
            double radDegree = toRad(m_degree);
            Point newPos = m_pos + Vector{cos(radDegree), -sin(radDegree)} * len;
            if (m_drawing) m_file.addLine(m_pos, newPos);
            m_pos = newPos;
        }

        void back(double len) {
            forward(-len);
        }

        void left(double angle, bool radians = false) {
            if (radians) {
                angle = toDeg(angle);
            }
            m_degree += angle;
        }

        void right(double angle, bool radians = false) {
            left(-angle, radians);
        }

        void drawing(bool draw) {
            m_drawing = draw;
        }

    private:
        double toRad(double degrees) {return M_PI/180 * degrees;}
        double toDeg(double radians) { return 180/M_PI * radians;}

        SVGFile m_file;
        Point m_pos;
        double m_degree = 0.0;
        bool m_drawing = true;
    };
};

//TODO possibly add lines at the end