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
#include <xeuclidean.h>
#include <dma.h>

using namespace cv;
using namespace cv::detail;
using namespace cv::cuda;

namespace {

unsigned int float_to_u32(float val){
	unsigned int result;
	union float_bytes{
		float v;
		unsigned char bytes[4];
	} data;
	data.v = val;
	result = ((data.bytes[3] << 24) + (data.bytes[2]<<16) + (data.bytes[1] << 8) + (data.bytes[0] << 0));
	return result;

}

} // namespace


namespace cv {
namespace detail {



void FpgaMatcher::bf_fpga_match(InputArray queryDescriptors, InputArray trainDescriptors,
                                  std::vector<DMatch> & matches) const {
    u32 * featureTable1Addr = (u32 *) 0xC0000000;
    u32 * featureTable2Addr = (u32 *) 0xC2000000;
    u32 * distancesTableAddr = (u32 *) 0xC4000000;
    u32 * indexTableAddr = (u32 *) 0x80000000;
    u32 * sendAddr  = (u32 *) 0xFFFF8000;

    volatile uint32_t * dmaMemInt = NULL;
    volatile float * dmaMemFloat = NULL;
    void * dmaMem  = NULL;
    volatile uint32_t * indexTable  = NULL;
    int fd = 0;

    XEuclidean euclidean_block;

    const int maxFeatures = 240;
    const int descSize = 32;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
      exit(-1);
    }

    // Map the DMA accesible memory
    dmaMem = mmap(NULL,
			maxFeatures * descSize * sizeof(uint32_t),
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			(uint32_t) sendAddr);
    if (dmaMem <= 0) {
      exit(-1);
    }

    dmaMemInt = (uint32_t *) dmaMem;
    dmaMemFloat = (float *) dmaMem;

		indexTable = &dmaMemInt[maxFeatures];

    // Configure DMA
    dmaInit();
    XEuclidean_Initialize(&euclidean_block, "euclidean");

		// std::cout << "First element" <<  << std::endl;
		// std::cout << "Full Matrix: " << queryDescriptors.getMat() << std::endl;
		// std::cout << "First element: " << (float)queryDescriptors.getMat().data[32] << std::endl;

    // DMA features table 1
		memcpy(dmaMem, queryDescriptors.getMat().data, maxFeatures * descSize * sizeof(uint32_t));
    dmaTransfer(sendAddr, featureTable1Addr, maxFeatures * descSize * sizeof(uint32_t));

    // DMA features table 2
    memcpy(dmaMem, trainDescriptors.getMat().data, maxFeatures * descSize * sizeof(uint32_t));
    dmaTransfer(sendAddr, featureTable2Addr, maxFeatures * descSize * sizeof(uint32_t));

    // Start Euclidean Block
    XEuclidean_Set_con(&euclidean_block, 1);
    XEuclidean_Set_tresh(&euclidean_block, float_to_u32(this->match_conf_));
    XEuclidean_Start(&euclidean_block);

    while(XEuclidean_IsDone(&euclidean_block) == 0){
      // Do nothing just busy waiting
  	}

    // DMA back the distances
    dmaTransfer(distancesTableAddr, sendAddr, maxFeatures * sizeof(uint32_t));
		dmaTransfer(indexTableAddr, (u32 *)((uint32_t)sendAddr +  maxFeatures * sizeof(uint32_t)), maxFeatures * sizeof(uint32_t));

    // Convert memory into matches
    for (int i = 0; i < maxFeatures; i++) {
      if (indexTable[i] == 0xFFFFFFFF) { // if it's 0xFFFF no match found
				// printf("OpenCV FPGA Matcher DEBUG: Wrong Index\n");
        continue;
      }

			// printf("OpenCV FPGA Matcher DEBUG: Match %d -> %d %f\n", i, indexTable[i], dmaMemFloat[i]);

      DMatch m(i, indexTable[i], dmaMemFloat[i]);
      matches.push_back(m);

    }

		// printf("OpenCV FPGA Matcher DEBUG: Matches %d", matches.size());
}


void FpgaMatcher::_match(const ImageFeatures &features1, const ImageFeatures &features2, MatchesInfo& matches_info)
{
    CV_Assert(features1.descriptors.type() == features2.descriptors.type());
    // CV_Assert(features2.descriptors.depth() == CV_32F);

		Mat f1, f2;

		features1.descriptors.convertTo(f1, CV_32F);
		features2.descriptors.convertTo(f2, CV_32F);

    matches_info.matches.clear();

    std::vector<DMatch> pair_matches;

    // Find matches
    bf_fpga_match(f1, f2, pair_matches);

		matches_info.matches.insert(matches_info.matches.end(), pair_matches.begin(), pair_matches.end());
}


//////////////////////////////////////////////////////////////////////////////

FpgaMatcher::FpgaMatcher(bool try_use_gpu, float match_conf, int num_matches_thresh1, int num_matches_thresh2)
{
    (void)try_use_gpu;

    // impl_ = makePtr<FpgaHWMatcher>(match_conf);

    is_thread_safe_ = true;
    num_matches_thresh1_ = num_matches_thresh1;
    num_matches_thresh2_ = num_matches_thresh2;
		this->match_conf_ = match_conf;
}

void FpgaMatcher::setThreshold(int th) {
	this->match_conf_ = th;
}

void FpgaMatcher::match(const ImageFeatures &features1, const ImageFeatures &features2,
                                  MatchesInfo &matches_info)
{
    _match(features1, features2, matches_info);

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
    // impl_->collectGarbage();
}


} // namespace detail
} // namespace cv
