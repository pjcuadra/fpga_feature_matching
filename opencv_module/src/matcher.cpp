/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"

using namespace cv;
using namespace cv::detail;
using namespace cv::cuda;

namespace {


// This class is aimed to find features matches only, not to
// estimate homography

class CpuMatcher : public FeaturesMatcher
{
public:
    CpuMatcher(float match_conf) : FeaturesMatcher(true), match_conf_(match_conf) {}
    void match(const ImageFeatures &features1, const ImageFeatures &features2, MatchesInfo& matches_info);

private:
    float match_conf_;
};


void CpuMatcher::match(const ImageFeatures &features1, const ImageFeatures &features2, MatchesInfo& matches_info)
{
    CV_Assert(features1.descriptors.type() == features2.descriptors.type());
    CV_Assert(features2.descriptors.depth() == CV_8U || features2.descriptors.depth() == CV_32F);

    matches_info.matches.clear();

    Ptr<cv::DescriptorMatcher> matcher;
    {
        Ptr<flann::IndexParams> indexParams = makePtr<flann::KDTreeIndexParams>();
        Ptr<flann::SearchParams> searchParams = makePtr<flann::SearchParams>();

        if (features2.descriptors.depth() == CV_8U)
        {
            indexParams->setAlgorithm(cvflann::FLANN_INDEX_LSH);
            searchParams->setAlgorithm(cvflann::FLANN_INDEX_LSH);
        }

        matcher = makePtr<FlannBasedMatcher>(indexParams, searchParams);
    }
    std::vector< std::vector<DMatch> > pair_matches;
    MatchesSet matches;

    // Find 1->2 matches
    matcher->knnMatch(features1.descriptors, features2.descriptors, pair_matches, 2);
    for (size_t i = 0; i < pair_matches.size(); ++i)
    {
        if (pair_matches[i].size() < 2)
            continue;
        const DMatch& m0 = pair_matches[i][0];
        const DMatch& m1 = pair_matches[i][1];
        if (m0.distance < (1.f - match_conf_) * m1.distance)
        {
            matches_info.matches.push_back(m0);
            matches.insert(std::make_pair(m0.queryIdx, m0.trainIdx));
        }
    }
    LOG("\n1->2 matches: " << matches_info.matches.size() << endl);

    // Find 2->1 matches
    pair_matches.clear();
    matcher->knnMatch(features2.descriptors, features1.descriptors, pair_matches, 2);
    for (size_t i = 0; i < pair_matches.size(); ++i)
    {
        if (pair_matches[i].size() < 2)
            continue;
        const DMatch& m0 = pair_matches[i][0];
        const DMatch& m1 = pair_matches[i][1];
        if (m0.distance < (1.f - match_conf_) * m1.distance)
            if (matches.find(std::make_pair(m0.trainIdx, m0.queryIdx)) == matches.end())
                matches_info.matches.push_back(DMatch(m0.trainIdx, m0.queryIdx, m0.distance));
    }
    LOG("1->2 & 2->1 matches: " << matches_info.matches.size() << endl);
}


} // namespace


namespace cv {
namespace detail {


//////////////////////////////////////////////////////////////////////////////

FpgaMatcher::FpgaMatcher(bool try_use_gpu, float match_conf, int num_matches_thresh1, int num_matches_thresh2)
{
    (void)try_use_gpu;

    impl_ = makePtr<CpuMatcher>(match_conf);

    is_thread_safe_ = impl_->isThreadSafe();
    num_matches_thresh1_ = num_matches_thresh1;
    num_matches_thresh2_ = num_matches_thresh2;
}


void FpgaMatcher::match(const ImageFeatures &features1, const ImageFeatures &features2,
                                  MatchesInfo &matches_info)
{
    (*impl_)(features1, features2, matches_info);

    // Check if it makes sense to find homography
    if (matches_info.matches.size() < static_cast<size_t>(num_matches_thresh1_))
        return;

    // Construct point-point correspondences for homography estimation
    Mat src_points(1, static_cast<int>(matches_info.matches.size()), CV_32FC2);
    Mat dst_points(1, static_cast<int>(matches_info.matches.size()), CV_32FC2);
    for (size_t i = 0; i < matches_info.matches.size(); ++i)
    {
        const DMatch& m = matches_info.matches[i];

        Point2f p = features1.keypoints[m.queryIdx].pt;
        p.x -= features1.img_size.width * 0.5f;
        p.y -= features1.img_size.height * 0.5f;
        src_points.at<Point2f>(0, static_cast<int>(i)) = p;

        p = features2.keypoints[m.trainIdx].pt;
        p.x -= features2.img_size.width * 0.5f;
        p.y -= features2.img_size.height * 0.5f;
        dst_points.at<Point2f>(0, static_cast<int>(i)) = p;
    }

    // Find pair-wise motion
    matches_info.H = findHomography(src_points, dst_points, matches_info.inliers_mask, RANSAC);
    if (matches_info.H.empty() || std::abs(determinant(matches_info.H)) < std::numeric_limits<double>::epsilon())
        return;

    // Find number of inliers
    matches_info.num_inliers = 0;
    for (size_t i = 0; i < matches_info.inliers_mask.size(); ++i)
        if (matches_info.inliers_mask[i])
            matches_info.num_inliers++;

    // These coeffs are from paper M. Brown and D. Lowe. "Automatic Panoramic Image Stitching
    // using Invariant Features"
    matches_info.confidence = matches_info.num_inliers / (8 + 0.3 * matches_info.matches.size());

    // Set zero confidence to remove matches between too close images, as they don't provide
    // additional information anyway. The threshold was set experimentally.
    matches_info.confidence = matches_info.confidence > 3. ? 0. : matches_info.confidence;

    // Check if we should try to refine motion
    if (matches_info.num_inliers < num_matches_thresh2_)
        return;

    // Construct point-point correspondences for inliers only
    src_points.create(1, matches_info.num_inliers, CV_32FC2);
    dst_points.create(1, matches_info.num_inliers, CV_32FC2);
    int inlier_idx = 0;
    for (size_t i = 0; i < matches_info.matches.size(); ++i)
    {
        if (!matches_info.inliers_mask[i])
            continue;

        const DMatch& m = matches_info.matches[i];

        Point2f p = features1.keypoints[m.queryIdx].pt;
        p.x -= features1.img_size.width * 0.5f;
        p.y -= features1.img_size.height * 0.5f;
        src_points.at<Point2f>(0, inlier_idx) = p;

        p = features2.keypoints[m.trainIdx].pt;
        p.x -= features2.img_size.width * 0.5f;
        p.y -= features2.img_size.height * 0.5f;
        dst_points.at<Point2f>(0, inlier_idx) = p;

        inlier_idx++;
    }

    // Rerun motion estimation on inliers only
    matches_info.H = findHomography(src_points, dst_points, RANSAC);
}

void FpgaMatcher::collectGarbage()
{
    impl_->collectGarbage();
}


} // namespace detail
} // namespace cv
