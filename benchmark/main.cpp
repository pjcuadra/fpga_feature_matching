/*
 * MIT License
 *
 * Copyright (c) 2017 Pedro Cuadra
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include <iostream>

// External
#include <opencv2/opencv.hpp>
#include <opencv2/stitching/detail/matchers.hpp>
#include <opencv2/fpga_matcher/matcher.hpp>

using namespace cv;
using namespace cv::detail;
using namespace std;

#define IMG_1 0
#define IMG_2 1
#define IMG_MAX 2

static String keys = "{help h usage ? |      | Print this message    }"
                     "{img1           |      | Image 1               }"
                     "{img2           |      | Image 2               }";

const string featuresFile("features.yml");
static const float	match_conf = 0.66f;

// Global variables
static Ptr<CommandLineParser> parser;
static Ptr<FeaturesFinder> finder;


// SW Matcher
static BestOf2NearestMatcher	matcherSw(false,	match_conf);
static vector<MatchesInfo> pairwiseMatchesSw;
// HW Matcher
static FpgaMatcher	matcherHw(false,	match_conf);
static vector<MatchesInfo> pairwiseMatchesHw;

void readImage(Mat &image, string path) {
  image = imread(path, IMREAD_COLOR);
  image.convertTo(image, CV_8UC3);
  assert(!image.empty());
}

int main(int argc, char **argv) {
  Mat images[IMG_MAX];
  Mat matchesImagesSw, matchesImagesHw;
  Mat bwImage;
  vector<ImageFeatures> features(IMG_MAX);
  double t = (double)getTickCount();

  parser = makePtr<CommandLineParser>(argc, argv, keys);

  // Read images
  cout << "Reading Image 1: " <<parser->get<string>("img1") << endl;
  readImage(images[IMG_1], parser->get<string>("img1"));
  cout << "Reading Image 2: " <<parser->get<string>("img2") << endl;
  readImage(images[IMG_2], parser->get<string>("img2"));

  // Create Feature Finder
  finder = makePtr<SurfFeaturesFinder>();

  imwrite("test.jpg", images[0]);

  // Find features
  for (int i = 0; i < IMG_MAX; i++) {
    (*finder)(images[i],	features[i]);
  }

  // Match using SW matcher
  matcherSw(features,	pairwiseMatchesSw);
  matcherSw.collectGarbage();

  // Match using HW matcher
  matcherHw(features,	pairwiseMatchesHw);
  matcherHw.collectGarbage();

  drawMatches(images[IMG_1],
              features[IMG_1].keypoints,
              images[IMG_2],
              features[IMG_2].keypoints,
              pairwiseMatchesSw[1].matches,
              matchesImagesSw);

  imwrite("matchesSW.jpg", matchesImagesSw);

  drawMatches(images[IMG_1],
              features[IMG_1].keypoints,
              images[IMG_2],
              features[IMG_2].keypoints,
              pairwiseMatchesHw[1].matches,
              matchesImagesHw);

  imwrite("matchesHW.jpg", matchesImagesHw);

  return 0;
}
