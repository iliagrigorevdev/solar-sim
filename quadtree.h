#ifndef QUADTREE_H
#define QUADTREE_H

#include <memory>
#include <vector>

class CelestialBody;

// Границы квадранта
struct Boundary {
  float x, y;      // центр
  float half_dim;  // половина размера
};

class Quadtree {
 public:
  Quadtree(const Boundary& boundary, int capacity);

  void insert(CelestialBody* body);
  void query(const Boundary& range, std::vector<CelestialBody*>& found);
  void clear();

  void compute_mass_distribution();
  void calculate_force(CelestialBody& body, float theta, float G,
                       float softening_factor);

  // Для отладки
  const Boundary& get_boundary() const { return boundary_; }
  const std::vector<std::unique_ptr<Quadtree>>& get_children() const;

 private:
  Boundary boundary_;
  int capacity_;
  std::vector<CelestialBody*> bodies_;
  bool divided_ = false;

  float total_mass_ = 0.0f;
  float center_of_mass_x_ = 0.0f;
  float center_of_mass_y_ = 0.0f;

  std::unique_ptr<Quadtree> northwest_;
  std::unique_ptr<Quadtree> northeast_;
  std::unique_ptr<Quadtree> southwest_;
  std::unique_ptr<Quadtree> southeast_;

  void subdivide();
};

#endif  // QUADTREE_H
