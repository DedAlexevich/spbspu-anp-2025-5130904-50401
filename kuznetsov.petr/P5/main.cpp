#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace kuznetsov {
  struct point_t {
    double x, y;
  };

  bool operator==(const point_t& a, const point_t& b)
  {
    return a.x == b.x && a.y == b.y;
  }
  bool operator!=(const point_t& a, const point_t& b)
  {
    return !(a == b);
  }

  struct rectangle_t {
    double width, height;
    point_t pos;
  };

  struct Shape {
    virtual double getArea() const = 0;
    virtual rectangle_t getFrameRect() const = 0;
    virtual void move(point_t p) = 0;
    virtual void move(double dx, double dy) = 0;
    virtual void scale(double m) = 0;
    virtual ~Shape() = default;
  };

  class Rectangle: public Shape {
    double width_, height_;
    point_t center_;
  public:
    Rectangle(double w, double h, point_t c);
    double getArea() const override;
    rectangle_t getFrameRect() const override;
    void move(point_t p) override;
    void move(double dx, double dy) override;
    void scale(double m) override;
  };

  class Triangle: public Shape {
    point_t a_, b_, c_;
    point_t center_;
  public:
    Triangle(point_t a, point_t b, point_t c);
    double getArea() const override;
    rectangle_t getFrameRect() const override;
    void move(point_t p) override;
    void move(double dx, double dy) override;
    void scale(double m) override;
  };

  class Square: public Rectangle {
  public:
    Square(double len, point_t c);
  };

  void scaleByPnt(Shape** fs, size_t size, point_t p, double m);
  double getSumArea(Shape** array, size_t size);
  rectangle_t getGenericFrame(Shape** array, size_t size);
  void print(Shape** fs, size_t s);

}

int main()
{
  namespace kuz = kuznetsov;
  kuz::Shape* figs[3] {nullptr, nullptr, nullptr};
  size_t size = 3;
  int statusCode = 0;

  double m = 0.0;
  kuz::point_t p {};
  std::cout << "x, y, scale: ";
  std::cin >> p.x >> p.y >> m;
  if (!std::cin || m <= 0) {
    std::cerr << "Incorrect enter\n";
    return 1;
  }

  try {
    figs[0] = new kuz::Rectangle(3.0, 5.0, {10, 3});
    figs[1] = new kuz::Triangle({10, 1}, {12, 5}, {8, 3});
    figs[2] = new kuz::Square(5, {10, 3});
  } catch (const std::bad_alloc& e) {
    std::cerr << e.what() << '\n';
    statusCode = 1;
  } catch (const std::invalid_argument& e) {
    std::cerr << e.what() << '\n';
    statusCode = 1;
  }

  if (statusCode == 0) {
    std::cout << "Before:\n";
    kuz::print(figs, size);
    kuz::scaleByPnt(figs, size, p, m);
    std::cout << "After:\n";
    kuz::print(figs, size);
  }

  delete figs[0];
  delete figs[1];
  delete figs[2];
  return statusCode;
}

kuznetsov::Rectangle::Rectangle(double w, double h, point_t c):
  width_(w),
  height_(h),
  center_(c)
{
  if (w <= 0 || h <= 0) {
    throw std::invalid_argument("Invalid size");
  }
}

double kuznetsov::Rectangle::getArea() const
{
  return width_ * height_;
}

kuznetsov::rectangle_t kuznetsov::Rectangle::getFrameRect() const
{
  return {width_, height_, center_};
}

void kuznetsov::Rectangle::move(point_t p)
{
  center_ = p;
}

void kuznetsov::Rectangle::move(double dx, double dy)
{
  center_.x += dx;
  center_.y += dy;
}

void kuznetsov::Rectangle::scale(double m)
{
  width_ *= m;
  height_ *= m;
}

void kuznetsov::scaleByPnt(Shape** fs, size_t size, point_t p, double m)
{
  if (!size || fs == nullptr) {
    throw std::invalid_argument("Empty array of shapes");
  }
  for (size_t i = 0; i < size; ++i) {
    point_t c = fs[i]->getFrameRect().pos;
    double dx = (c.x - p.x) * (m - 1);
    double dy = (c.y - p.y) * (m - 1);
    fs[i]->move(dx, dy);
    fs[i]->scale(m);
  }
}

double kuznetsov::getSumArea(Shape** array, size_t size)
{
  if (!size || array == nullptr) {
    throw std::invalid_argument("Empty array of shapes");
  }
  double finalArea = 0.0;
  for (size_t i = 0; i < size; ++i) {
    finalArea += array[i]->getArea();
  }
  return finalArea;
}

kuznetsov::rectangle_t kuznetsov::getGenericFrame(Shape** array, size_t size)
{
  if (!size || array == nullptr) {
    throw std::invalid_argument("Empty array of shapes");
  }
  rectangle_t genericFrame{};
  rectangle_t fr = array[0]->getFrameRect();
  double maxY = fr.pos.y + fr.height / 2;
  double minY = fr.pos.y - fr.height / 2;
  double maxX = fr.pos.x + fr.width / 2;
  double minX = fr.pos.x - fr.width / 2;

  for (size_t i = 1; i < size; ++i) {
    Shape* f = array[i];
    fr = f->getFrameRect();
    maxY = std::max(maxY, fr.pos.y + fr.height / 2);
    maxX = std::max(maxX, fr.pos.x + fr.width / 2);
    minY = std::min(minY, fr.pos.y - fr.height / 2);
    minX = std::min(minX, fr.pos.x - fr.width / 2);
  }

  double cx = (maxX + minX) / 2;
  double cy = (maxY + minY) / 2;
  genericFrame.width = maxX - minX;
  genericFrame.height = maxY - minY;
  genericFrame.pos = {cx, cy};
  return genericFrame;
}

kuznetsov::Triangle::Triangle(point_t a, point_t b, point_t c):
  a_(a),
  b_(b),
  c_(c),
  center_{}
{
  bool incorrect = a_ == b_;
  incorrect = incorrect || a_ == c_;
  incorrect = incorrect || c_ == b_;
  bool oneLine = (b_.y - a_.y) * (c_.x - a_.x) == (c_.y - a_.y) * (b_.x - a_.x);
  incorrect = incorrect || oneLine;
  if (incorrect) {
    throw std::invalid_argument("Invalid vertexes, it's line");
  }
  double x = (a_.x + b_.x + c_.x) / 3;
  double y = (a_.y + b_.y + c_.y) / 3;
  center_ = {x, y};
}

double kuznetsov::Triangle::getArea() const
{
  double s = a_.x * (b_.y - c_.y);
  s = b_.x * (c_.y - a_.y) + s;
  s = c_.x * (a_.y - b_.y) + s;
  s = 0.5 * std::abs(s);
  return s;
}

kuznetsov::rectangle_t kuznetsov::Triangle::getFrameRect() const
{
  rectangle_t frame{};
  double maxX = std::max(a_.x, std::max(b_.x, c_.x));
  double minX = std::min(a_.x, std::min(b_.x, c_.x));
  double maxY = std::max(a_.y, std::max(b_.y, c_.y));
  double minY = std::min(a_.y, std::min(b_.y, c_.y));
  double cx = (maxX + minX) / 2;
  double cy = (maxY + minY) / 2;
  frame.height = maxY - minY;
  frame.width = maxX - minX;
  frame.pos = {cx, cy};
  return frame;
}

void kuznetsov::Triangle::move(point_t p)
{
  double dx = p.x - center_.x;
  double dy = p.y - center_.y;
  move(dx, dy);
}

void kuznetsov::Triangle::move(double dx, double dy)
{
  a_.x += dx;
  a_.y += dy;

  b_.x += dx;
  b_.y += dy;

  c_.x += dx;
  c_.y += dy;

  center_.x += dx;
  center_.y += dy;
}

void kuznetsov::Triangle::scale(double m)
{
  point_t dpa {a_.x - center_.x , a_.y - center_.y};
  point_t dpb {b_.x - center_.x , b_.y - center_.y};
  point_t dpc {c_.x - center_.x , c_.y - center_.y};

  a_.x = center_.x + dpa.x * m;
  a_.y = center_.y + dpa.y * m;

  b_.x = center_.x + dpb.x * m;
  b_.y = center_.y + dpb.y * m;

  c_.x = center_.x + dpc.x * m;
  c_.y = center_.y + dpc.y * m;
}

kuznetsov::Square::Square(double len, point_t c):
  Rectangle(len, len, c)
{}

void kuznetsov::print(Shape** fs, size_t s)
{
  for (size_t i = 0; i < s; ++i) {
    rectangle_t fr = fs[i]->getFrameRect();
    std::cout << "Figure " << i << ":\n";
    std::cout << "\tArea: " << fs[i]->getArea();
    std::cout << "\n\tFrame rectangle: ";
    std::cout << "\n\t\tWidth: " << fr.width;
    std::cout << "\n\t\tHeight: " << fr.height;
    std::cout << "\n\t\tCenter: " << "x = " << fr.pos.x;
    std::cout << " " << "y = " << fr.pos.y << '\n';
  }
  std::cout << "SumArea: " << getSumArea(fs, s);
  rectangle_t genFr = getGenericFrame(fs, s);
  std::cout << "\nGeneric frame: ";
  std::cout << "\n\tWidth: " << genFr.width;
  std::cout << "\n\tHeight: " << genFr.height;
  std::cout << "\n\tCenter: " << "x = " << genFr.pos.x;
  std::cout << " " << "y = " << genFr.pos.y << '\n';
}
