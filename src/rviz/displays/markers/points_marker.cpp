/*
 * Copyright (c) 2009, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "points_marker.h"
#include "common.h"
#include "visualization_manager.h"

#include <ogre_tools/point_cloud.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

namespace rviz
{

PointsMarker::PointsMarker(VisualizationManager* manager, Ogre::SceneNode* parent_node)
: MarkerBase(manager, parent_node)
, points_(0)
{
  scene_node_ = vis_manager_->getSceneManager()->getRootSceneNode()->createChildSceneNode();
}

PointsMarker::~PointsMarker()
{
  vis_manager_->getSceneManager()->destroySceneNode(scene_node_->getName());
  delete points_;
}

void PointsMarker::onNewMessage(const MarkerPtr& old_message, const MarkerPtr& new_message)
{
  ROS_ASSERT(new_message->type == visualization_msgs::Marker::POINTS);

  if (!points_)
  {
    points_ = new ogre_tools::PointCloud(vis_manager_->getSceneManager(), parent_node_);
  }

  points_->setBillboardDimensions(new_message->scale.x, new_message->scale.y);

  Ogre::Vector3 pos, scale;
  Ogre::Quaternion orient;
  transform(new_message, pos, orient, scale);

  scene_node_->setPosition(pos);
  scene_node_->setOrientation(orient);

  if (new_message->points.empty())
  {
    ROS_ERROR("Marker [%s/%d] has no points!", new_message->ns.c_str(), new_message->id);
    return;
  }

  float r = new_message->color.r;
  float g = new_message->color.g;
  float b = new_message->color.b;
  float a = new_message->color.a;
  points_->setAlpha(a);

  points_->clear();

  typedef std::vector< ogre_tools::PointCloud::Point > V_Point;
  V_Point points;
  points.resize(new_message->points.size());
  std::vector<robot_msgs::Point>::iterator it = new_message->points.begin();
  std::vector<robot_msgs::Point>::iterator end = new_message->points.end();
  for (int i = 0; it != end; ++it, ++i)
  {
    robot_msgs::Point& p = *it;
    ogre_tools::PointCloud::Point& point = points[i];

    Ogre::Vector3 v(p.x, p.y, p.z);
    robotToOgre(v);

    point.x_ = v.x;
    point.y_ = v.y;
    point.z_ = v.z;
    point.r_ = r;
    point.g_ = g;
    point.b_ = b;
  }

  points_->addPoints(&points.front(), points.size());
}

}