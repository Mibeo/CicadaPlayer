cmake_minimum_required(VERSION 3.6)

project(CicadaPlayerSDK)

set(TOP_DIR "${CMAKE_SOURCE_DIR}/../../..")


if (IOS)
    set(TARGET_PLATFORM iOS)
    set(PATH_NAME iOS)
else ()
    set(TARGET_PLATFORM macOSX)
    set(PATH_NAME Darwin)
endif ()
include(config.cmake)

set(CMAKE_CXX_STANDARD 11)

set(ENV{DEPLOYMENT_TARGET} "8.0")

set(IPHONEOS_DEPLOYMENT_TARGET $ENV{DEPLOYMENT_TARGET})
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET $ENV{DEPLOYMENT_TARGET})
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
set(ALIBRARYS ${ALI_SRC_LIBRARIES})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
add_subdirectory(${TOP_DIR}/mediaPlayer mediaPlayer.out)
add_subdirectory(${TOP_DIR}/external/install/ffmpeg/${PATH_NAME} alivcffmpeg.out)
add_subdirectory(${TOP_DIR}/plugin plugin.out)

add_compile_options(-x objective-c++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-abi-version=2 -fobjc-arc")

include_directories(
        ./thumbnail
        ${TOP_DIR}/mediaPlayer
        ${TOP_DIR}/framework
        ${TOP_DIR}/framework/render/video/glRender/platform/ios)

link_directories(${SRC_LIBRARIES_DIR})

set(SOURCE_LIST
        CicadaPlayer.mm
        CicadaPlayer.h
        CicadaPlayerSDK.h
        CicadaConfig.mm
        CicadaConfig.h
        CicadaCacheConfig.mm
        CicadaCacheConfig.h
        CicadaDef.mm
        CicadaDef.h
        CicadaOCHelper.mm
        CicadaOCHelper.h
        CicadaMediaInfo.mm
        CicadaMediaInfo.h
        CicadaSource.mm
        CicadaSource.h
        CicadaDelegate.h
        CicadaErrorCode.h
        thumbnail/CicadaThumbnail.h
        thumbnail/CicadaThumbnail.mm
        thumbnail/CicadaThumbnailItem.h
        thumbnail/CicadaThumbnailItem.mm
        thumbnail/CicadaURLSession.h
        thumbnail/CicadaURLSession.mm)

add_library(CicadaPlayerSDK SHARED
        ${SOURCE_LIST}
        )
message("LIBRARYS is ${ALIBRARYS}")
target_link_libraries(CicadaPlayerSDK PRIVATE ${ALIBRARYS})
target_link_libraries(CicadaPlayerSDK PUBLIC alivcffmpeg)

target_include_directories(CicadaPlayerSDK PUBLIC ${PROJECT_SOURCE_DIR})

set_source_files_properties(${XIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set(PUBLIC_HEADERS
        CicadaPlayer.h
        CicadaConfig.h
        CicadaCacheConfig.h
        CicadaDef.h
        CicadaMediaInfo.h
        CicadaSource.h
        CicadaPlayerSDK.h
        CicadaDelegate.h
        CicadaErrorCode.h
        )

if (IOS)
    set(BITCODE_VALUE "YES")
    set(BITCODE_GENERATION_VALUE bitcode)
else ()
    set(BITCODE_VALUE "NO")
    set(BITCODE_GENERATION_VALUE marker)
endif ()

set_target_properties(CicadaPlayerSDK PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.alibaba.CicadaPlayerSDK
        LINK_FLAGS "-Wl,${LINK_OPTS}"
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
        LINKER_LANGUAGE C
        MACOSX_FRAMEWORK_BUNDLE_VERSION 1
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING 1.0
        INSTALL_NAME_DIR @rpath
        #        MACOSX_FRAMEWORK_INFO_PLIST Info.plist
        #XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=MinSizeRel] YES
        XCODE_ATTRIBUTE_ENABLE_BITCODE ${BITCODE_VALUE}
        XCODE_ATTRIBUTE_DEFINES_MODULE "YES"
        XCODE_ATTRIBUTE_BITCODE_GENERATION_MODE[variant=MinSizeRel] ${BITCODE_GENERATION_VALUE}
        )

add_custom_command(
        TARGET CicadaPlayerSDK
        POST_BUILD
        COMMAND install_name_tool -id \"@rpath/CicadaPlayerSDK.framework/CicadaPlayerSDK\"
        $<TARGET_FILE:CicadaPlayerSDK>
)

set(INSTALL_TARGETS CicadaPlayerSDK)
#DON_T change the install lib's order
if (IOS)
    set(INSTALL_TARGETS ${INSTALL_TARGETS}
            alivcffmpeg
            CicadaPlayerSDK
            )

    install(TARGETS ${INSTALL_TARGETS}
            FRAMEWORK DESTINATION
            ${PROJECT_BINARY_DIR}/ARM_SIMULATOR)

#    add_custom_command(TARGET CicadaPlayerSDK POST_BUILD
#    COMMAND cp -R ${TOP_DIR}/prebuild/${PATH_NAME}/Conan/AlivcConan.framework ${PROJECT_BINARY_DIR}/ARM_SIMULATOR
#    )
else ()
    set(INSTALL_TARGETS ${INSTALL_TARGETS}
            alivcffmpeg
            CicadaPlayerSDK
            )

    install(TARGETS ${INSTALL_TARGETS}
            FRAMEWORK DESTINATION
            ${PROJECT_BINARY_DIR})
    endif()

if (IOS)
#    add_custom_command(TARGET CicadaPlayerSDK POST_BUILD
#    COMMAND cp -R ${TOP_DIR}/prebuild/${PATH_NAME}/Conan/AlivcConan.framework ${PROJECT_BINARY_DIR}
#    )
else ()
#    add_custom_command(TARGET CicadaPlayerSDK POST_BUILD
#    COMMAND sh ../copyFramework.sh
#    )
endif ()
