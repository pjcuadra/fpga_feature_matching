set(the_description "FPGA Matcher")

if(HAVE_CUDA)
  ocv_warnings_disable(CMAKE_CXX_FLAGS -Wundef -Wmissing-declarations -Wshadow)
endif()

set(STITCHING_CONTRIB_DEPS "opencv_xfeatures2d")
if(BUILD_SHARED_LIBS AND BUILD_opencv_world)
  set(STITCHING_CONTRIB_DEPS "")
endif()
ocv_define_module(fpga_matcher opencv_imgproc opencv_features2d opencv_calib3d opencv_objdetect
                  OPTIONAL opencv_cudaarithm opencv_cudafilters opencv_cudafeatures2d opencv_cudalegacy ${STITCHING_CONTRIB_DEPS}
                  WRAP python)
