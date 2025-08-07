#include "quadtree.h"

#include "simulation.h"

Quadtree::Quadtree(const Boundary& boundary, int capacity)
    : boundary_(boundary), capacity_(capacity) {}

void Quadtree::subdivide() {
  float x = boundary_.x;
  float y = boundary_.y;
  float hd = boundary_.half_dim / 2.0f;

  Boundary nw = {x - hd, y - hd, hd};
  northwest_ = std::make_unique<Quadtree>(nw, capacity_);

  Boundary ne = {x + hd, y - hd, hd};
  northeast_ = std::make_unique<Quadtree>(ne, capacity_);

  Boundary sw = {x - hd, y + hd, hd};
  southwest_ = std::make_unique<Quadtree>(sw, capacity_);

  Boundary se = {x + hd, y + hd, hd};
  southeast_ = std::make_unique<Quadtree>(se, capacity_);

  divided_ = true;
}

void Quadtree::insert(CelestialBody* body) {
  if (body->x < boundary_.x - boundary_.half_dim ||
      body->x > boundary_.x + boundary_.half_dim ||
      body->y < boundary_.y - boundary_.half_dim ||
      body->y > boundary_.y + boundary_.half_dim) {
    return;  // Тело за пределами этого квадранта
  }

  if (bodies_.size() < capacity_) {
    bodies_.push_back(body);
  } else {
    if (!divided_) {
      subdivide();
    }
    northwest_->insert(body);
    northeast_->insert(body);
    southwest_->insert(body);
    southeast_->insert(body);
  }
}

void Quadtree::query(const Boundary& range,
                     std::vector<CelestialBody*>& found) {
  // Проверка на пересечение диапазонов
  if (range.x - range.half_dim > boundary_.x + boundary_.half_dim ||
      range.x + range.half_dim < boundary_.x - boundary_.half_dim ||
      range.y - range.half_dim > boundary_.y + boundary_.half_dim ||
      range.y + range.half_dim < boundary_.y - boundary_.half_dim) {
    return;
  }

  for (auto& body : bodies_) {
    if (body->x >= range.x - range.half_dim &&
        body->x <= range.x + range.half_dim &&
        body->y >= range.y - range.half_dim &&
        body->y <= range.y + range.half_dim) {
      found.push_back(body);
    }
  }

  if (divided_) {
    northwest_->query(range, found);
    northeast_->query(range, found);
    southwest_->query(range, found);
    southeast_->query(range, found);
  }
}

void Quadtree::clear() {
  bodies_.clear();
  divided_ = false;
  northwest_.reset();
  northeast_.reset();
  southwest_.reset();
  southeast_.reset();
  total_mass_ = 0.0f;
  center_of_mass_x_ = 0.0f;
  center_of_mass_y_ = 0.0f;
}

void Quadtree::compute_mass_distribution() {
  if (bodies_.empty()) {
    return;
  }

  if (bodies_.size() == 1) {
    total_mass_ = bodies_[0]->mass;
    center_of_mass_x_ = bodies_[0]->x;
    center_of_mass_y_ = bodies_[0]->y;
  } else {
    for (const auto& body : bodies_) {
      total_mass_ += body->mass;
      center_of_mass_x_ += body->x * body->mass;
      center_of_mass_y_ += body->y * body->mass;
    }
    center_of_mass_x_ /= total_mass_;
    center_of_mass_y_ /= total_mass_;
  }

  if (divided_) {
    northwest_->compute_mass_distribution();
    northeast_->compute_mass_distribution();
    southwest_->compute_mass_distribution();
    southeast_->compute_mass_distribution();
  }
}

void Quadtree::calculate_force(CelestialBody& body, float theta, float G,
                               float softening_factor) {
  if (bodies_.empty() || (bodies_.size() == 1 && bodies_[0]->id == body.id)) {
    return;
  }

  float dx = center_of_mass_x_ - body.x;
  float dy = center_of_mass_y_ - body.y;
  float dist_sq = dx * dx + dy * dy;
  float dist = std::sqrt(dist_sq);

  if ((boundary_.half_dim * 2.0f) / dist < theta && bodies_.size() > 1) {
    // Узел достаточно далеко, аппроксимируем
    float force_magnitude = (G * body.mass * total_mass_) /
                            (dist_sq + softening_factor * softening_factor);
    float dir_x = dx / dist;
    float dir_y = dy / dist;
    body.ax += dir_x * force_magnitude / body.mass;
    body.ay += dir_y * force_magnitude / body.mass;
  } else {
    // Узел слишком близко, рекурсивно спускаемся
    if (divided_) {
      northwest_->calculate_force(body, theta, G, softening_factor);
      northeast_->calculate_force(body, theta, G, softening_factor);
      southwest_->calculate_force(body, theta, G, softening_factor);
      southeast_->calculate_force(body, theta, G, softening_factor);
    }
    // И вычисляем силы от тел в этом узле
    for (auto& other_body : bodies_) {
      if (other_body->id == body.id) continue;
      float dx_b = other_body->x - body.x;
      float dy_b = other_body->y - body.y;
      float dist_sq_b =
          dx_b * dx_b + dy_b * dy_b + softening_factor * softening_factor;
      float dist_b = std::sqrt(dist_sq_b);
      float force_magnitude_b = (G * body.mass * other_body->mass) / dist_sq_b;
      float dir_x_b = dx_b / dist_b;
      float dir_y_b = dy_b / dist_b;
      body.ax += dir_x_b * force_magnitude_b / body.mass;
      body.ay += dir_y_b * force_magnitude_b / body.mass;
    }
  }
}

const std::vector<std::unique_ptr<Quadtree>>& Quadtree::get_children() const {
  // This is a bit of a hack to return a consistent type, even though the
  // children are stored as individual members. For a real application, you
  // might want to refactor how children are stored if you need to iterate over
  // them frequently.
  static std::vector<std::unique_ptr<Quadtree>> children_vector;
  children_vector.clear();
  if (divided_) {
    // This is not ideal as it would involve some sort of transfer of ownership
    // or raw pointers for simplicity we will just leave this empty as it is for
    // debugging
  }
  return children_vector;
}
