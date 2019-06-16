find_package(OpenCV REQUIRED)

# Define the current source locations
SET(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/grabber)
SET(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libsrc/grabber/opencv)

SET(OPENCV_GRABBER_QT_HEADERS
    ${CURRENT_HEADER_DIR}/OpenCVGrabber.h
    ${CURRENT_HEADER_DIR}/OpenCVWrapper.h
)

SET(OPENCV_GRABBER_SOURCES
    ${CURRENT_SOURCE_DIR}/OpenCVGrabber.cpp
    ${CURRENT_SOURCE_DIR}/OpenCVWrapper.cpp
)

if(ENABLE_QT5)
    QT5_WRAP_CPP(OPENCV_GRABBER_HEADERS_MOC ${OPENCV_GRABBER_QT_HEADERS})
else()
    QT4_WRAP_CPP(OPENCV_GRABBER_HEADERS_MOC ${OPENCV_GRABBER_QT_HEADERS})
endif()

add_library(opencv-grabber
    ${OPENCV_GRABBER_HEADERS}
    ${OPENCV_GRABBER_SOURCES}
    ${OPENCV_GRABBER_QT_HEADERS}
    ${OPENCV_GRABBER_HEADERS_MOC}
)

if(ENABLE_QT5)
qt5_use_modules(opencv-grabber Widgets)
endif(ENABLE_QT5)

target_link_libraries(opencv-grabber
    hyperion
    ${QT_LIBRARIES}
    ${OpenCV_LIBS}
)
