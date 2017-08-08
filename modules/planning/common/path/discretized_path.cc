/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file path.cc
 **/

#include "modules/planning/common/path/discretized_path.h"

#include <algorithm>
#include <utility>

#include "modules/common/log.h"
#include "modules/planning/common/planning_util.h"

namespace apollo {
namespace planning {

DiscretizedPath::DiscretizedPath(std::vector<common::PathPoint> path_points) {
  path_points_ = std::move(path_points);
}

void DiscretizedPath::set_path_points(
    const std::vector<common::PathPoint> &path_points) {
  path_points_ = path_points;
}

common::PathPoint DiscretizedPath::Evaluate(const double path_s) const {
  CHECK_GT(path_points_.size(), 1);
  CHECK(path_points_.front().s() <= path_s &&
        path_points_.back().s() <= path_s);

  auto it_lower = QueryLowerBound(path_s);
  if (it_lower == path_points_.begin()) {
    return path_points_.front();
  }

  return util::interpolate(*(it_lower - 1), *it_lower, path_s);
}

double DiscretizedPath::Length() const {
  if (path_points_.empty()) {
    return 0.0;
  }
  return path_points_.back().s() - path_points_.front().s();
}

common::PathPoint DiscretizedPath::EvaluateUsingLinearApproximation(
    const double path_s) const {
  CHECK(!path_points_.empty());
  auto it_lower = QueryLowerBound(path_s);
  if (it_lower == path_points_.begin()) {
    return path_points_.front();
  }
  if (it_lower == path_points_.end()) {
    return path_points_.back();
  }
  return util::interpolate_linear_approximation(*(it_lower - 1), *it_lower,
                                                path_s);
}

int DiscretizedPath::QueryClosestPoint(const double path_s) const {
  if (path_points_.empty()) {
    return -1;
  }
  auto it_lower = QueryLowerBound(path_s);

  if (it_lower == path_points_.begin()) {
    return 0;
  }
  if (it_lower == path_points_.end()) {
    return path_points_.size() - 1;
  }

  double d0 = path_s - (it_lower - 1)->s();
  double d1 = it_lower->s() - path_s;

  if (d0 < d1) {
    return static_cast<int>(it_lower - path_points_.begin()) - 1;
  } else {
    return static_cast<int>(it_lower - path_points_.begin());
  }
}

const std::vector<common::PathPoint> &DiscretizedPath::path_points() const {
  return path_points_;
}

std::uint32_t DiscretizedPath::NumOfPoints() const {
  return path_points_.size();
}

const common::PathPoint &DiscretizedPath::PathPointAt(
    const std::uint32_t index) const {
  CHECK_LT(index, path_points_.size());
  return path_points_[index];
}

const common::PathPoint &DiscretizedPath::StartPoint() const {
  CHECK(!path_points_.empty());
  return path_points_.front();
}

const common::PathPoint &DiscretizedPath::EndPoint() const {
  CHECK(!path_points_.empty());
  return path_points_.back();
}

void DiscretizedPath::Clear() { path_points_.clear(); }

std::vector<common::PathPoint>::const_iterator DiscretizedPath::QueryLowerBound(
    const double path_s) const {
  auto func = [](const common::PathPoint &tp, const double path_s) {
    return tp.s() < path_s;
  };
  return std::lower_bound(path_points_.begin(), path_points_.end(), path_s,
                          func);
}

}  // namespace planning
}  // namespace apollo
