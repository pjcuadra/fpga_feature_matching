set(the_description "FPGA Matcher")

if(HAVE_CUDA)
  ocv_warnings_disable(CMAKE_CXX_FLAGS -Wundef -Wmissing-declarations -Wshadow)
endif()

set(FPGA_MATCHER_CONTRIB_DEPS "opencv_xfeatures2d")
if(BUILD_SHARED_LIBS AND BUILD_opencv_world)
  set(FPGA_MATCHER_CONTRIB_DEPS "")
endif()
ocv_define_module(fpga_matcher opencv_stitching opencv_imgproc opencv_features2d opencv_calib3d opencv_objdetect
                  xeuclidean dma OPTIONAL opencv_cudaarithm opencv_cudafilters opencv_cudafeatures2d opencv_cudalegacy ${FPGA_MATCHER_CONTRIB_DEPS}
                  WRAP python)
