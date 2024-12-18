// Copyright (c) 2019 CNRS
// Authors: Joseph Mirabel
//
//
// This file is part of hpp-gui
// hpp-gui is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-gui is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-gui  If not, see
// <http://www.gnu.org/licenses/>.

#include <coal/mesh_loader/assimp.h>

#include <node.hh>

namespace hpp {
namespace gui {
BVHDisplay::BVHDisplay(const std::string& filename, const std::string& name)
    : Node(name), filename_(filename) {
  setWireFrameMode(gepetto::viewer::WIREFRAME);
}

void BVHDisplay::setColor(const osgVector4&) {
  // TODO
}

void BVHDisplay::setLevel(const int& level) {
  if (level >= (int)levels_.size())
    throw std::invalid_argument("level out of range");
  this->asQueue()->replaceChild(levels_[level_].geode, levels_[level].geode);
  level_ = level;
}

void BVHDisplay::init(coal::SplitMethodType splitMethod) {
  using namespace coal;

  BVHPtr_t bvh(new BVH_t);
  bvh->bv_splitter.reset(new BVSplitter<BoundingVolume>(splitMethod));
  loadPolyhedronFromResource(filename_, Vec3s(1, 1, 1), bvh);

  recursiveBuildTree(*bvh, 0, 0);

  level_ = 0;
  this->asQueue()->addChild(levels_[0].geode);

  using gepetto::viewer::RangedIntProperty;
  RangedIntProperty::Ptr_t levelProp = RangedIntProperty::create(
      "Level", this, &BVHDisplay::getLevel, &BVHDisplay::setLevel);
  levelProp->min = 0;
  levelProp->max = (int)(levels_.size() - 1);
  addProperty(levelProp);
}

void BVHDisplay::recursiveBuildTree(const BVH_t& bvh, int ibv,
                                    std::size_t level) {
  if (levels_.size() <= level) levels_.resize(level + 1);
  BVLevel& bvlevel = levels_[level];

  const BoundingVolume& bv = bvh.getBV(ibv).bv;
  ::osg::BoxRefPtr box = new ::osg::Box();
  box->setCenter(osg::Vec3((float)bv.To[0], (float)bv.To[1], (float)bv.To[2]));
  box->setHalfLengths(
      osg::Vec3((float)bv.extent[0], (float)bv.extent[1], (float)bv.extent[2]));
  Eigen::Quaterniond q(bv.axes);
  box->setRotation(::osg::Quat(q.x(), q.y(), q.z(), q.w()));

  ::osg::ShapeDrawableRefPtr drawable = new ::osg::ShapeDrawable(box);

  bvlevel.boxes.push_back(box);
  if (!bvlevel.geode) bvlevel.geode = new ::osg::Geode;
  bvlevel.geode->addDrawable(drawable);

  if (bvh.getBV(ibv).isLeaf()) return;
  recursiveBuildTree(bvh, bvh.getBV(ibv).leftChild(), level + 1);
  recursiveBuildTree(bvh, bvh.getBV(ibv).rightChild(), level + 1);
}
}  // namespace gui
}  // namespace hpp
