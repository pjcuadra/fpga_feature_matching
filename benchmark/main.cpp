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

using namespace cv;
using namespace cv::xfeatures2d;
using namespace cv::detail;
using namespace std;

static String keys = "{help h usage ? |      | Print this message    }"
                     "{img1           |      | Image 1               }"
                     "{img2           |      | Image 2               }";

const string featuresFile("features.yml");

// Global variables
static Ptr<CommandLineParser> parser;


void readImage(Mat &image, string path) {
  image = imread(path);
  assert(!image.empty());
}

int main(int argc, char **argv) {
  int skipped = 0;
  double t = (double)getTickCount();

  parser = makePtr<CommandLineParser>(argc, argv, keys);

  cout << "Params: " << endl;
  cout << parser.get<string>("img1") << endl;
  cout << parser.get<string>("img2") << endl;

  return 0;
}
