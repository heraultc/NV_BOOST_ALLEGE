#=============================================================================#
#  NV-BOOST ALLÉGÉ — build multiplateforme (Linux, Windows, macOS)
#
#  Remplace NightVector.pro (qmake). CMake est le seul système de build
#  qui fonctionne proprement avec GitHub Actions sur les 3 OS.
#
#  Usage local :
#     cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
#     cmake --build build -j
#=============================================================================#

cmake_minimum_required(VERSION 3.21)

project(NightVector
        VERSION 1.0.0
        DESCRIPTION "NV-BOOST allégé — rehaussement vidéo nocturne"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AUTOMOC : génère le code des signaux/slots Qt.
# AUTORCC : compile automatiquement resources.qrc.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Par défaut on compile en Release (sinon binaire lent + lourd).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# MSVC (Windows) : indispensable, les sources contiennent des accents UTF-8.
if(MSVC)
    add_compile_options(/utf-8 /permissive- /MP)
endif()

#-----------------------------------------------------------------------------
# Dépendances
#-----------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

# Pas de COMPONENTS ici : la build officielle Windows fournit un seul module
# fusionné (opencv_world), ce qui ferait échouer une demande par composants.
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV ${OpenCV_VERSION} trouvé dans ${OpenCV_DIR}")

qt_standard_project_setup()

#-----------------------------------------------------------------------------
# Cible
#-----------------------------------------------------------------------------
set(APP_SOURCES
    main.cpp
    backend.cpp
    videoprocessor.cpp
    translator.cpp

    backend.h
    framenotifier.h
    videoprocessor.h
    config.h
    translator.h

    resources.qrc          # QML + fichiers de langue + logo
)

qt_add_executable(NightVector ${APP_SOURCES})

set_target_properties(NightVector PROPERTIES
    WIN32_EXECUTABLE TRUE      # Windows : pas de fenêtre console noire
    MACOSX_BUNDLE    TRUE      # macOS  : produit NightVector.app
    OUTPUT_NAME      "NightVector"
)

target_include_directories(NightVector PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(NightVector PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    ${OpenCV_LIBS}
)

#-----------------------------------------------------------------------------
# macOS : Info.plist (autorisation caméra obligatoire depuis macOS 10.14)
#-----------------------------------------------------------------------------
if(APPLE)
    set_target_properties(NightVector PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST      "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME     "NV-BOOST"
        MACOSX_BUNDLE_EXECUTABLE_NAME "NightVector"
        MACOSX_BUNDLE_GUI_IDENTIFIER  "com.nightvector.nvboost"
        MACOSX_BUNDLE_BUNDLE_VERSION  "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()
