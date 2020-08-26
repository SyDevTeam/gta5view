cmake_minimum_required(VERSION 3.5)

project(gta5view LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Network Svg Widgets REQUIRED)

add_executable(gta5view
    config.h
    main.cpp
    AboutDialog.cpp
    AboutDialog.h
    AboutDialog.ui
    AppEnv.cpp
    AppEnv.h
    CrewDatabase.cpp
    CrewDatabase.h
    DatabaseThread.cpp
    DatabaseThread.h
    ExportDialog.cpp
    ExportDialog.h
    ExportThread.cpp
    ExportThread.h
    GlobalString.cpp
    GlobalString.h
    IconLoader.cpp
    IconLoader.h
    ImportDialog.cpp
    ImportDialog.h
    ImportDialog.ui
    JsonEditorDialog.cpp
    JsonEditorDialog.h
    JsonEditorDialog.ui
    MapLocationDialog.cpp
    MapLocationDialog.h
    MapLocationDialog.ui
    OptionsDialog.cpp
    OptionsDialog.h
    OptionsDialog.ui
    PictureDialog.cpp
    PictureDialog.h
    PictureDialog.ui
    PictureExport.cpp
    PictureExport.h
    PictureWidget.cpp
    PictureWidget.h
    PlayerListDialog.cpp
    PlayerListDialog.h
    PlayerListDialog.ui
    ProfileDatabase.cpp
    ProfileDatabase.h
    ProfileInterface.cpp
    ProfileInterface.h
    ProfileInterface.ui
    ProfileLoader.cpp
    ProfileLoader.h
    ProfileWidget.cpp
    ProfileWidget.h
    SavegameCopy.cpp
    SavegameCopy.h
    SavegameData.cpp
    SavegameData.h
    SavegameDialog.cpp
    SavegameDialog.h
    SavegameDialog.ui
    SavegameWidget.cpp
    SavegameWidget.h
    SavegameWidget.ui
    SidebarGenerator.cpp
    SidebarGenerator.h
    SnapmaticEditor.cpp
    SnapmaticEditor.h
    SnapmaticEditor.ui
    SnapmaticPicture.cpp
    SnapmaticPicture.h
    SnapmaticWidget.cpp
    SnapmaticWidget.h
    SnapmaticWidget.ui
    StandardPaths.cpp
    StandardPaths.h
    StringParser.cpp
    StringParser.h
    TranslationClass.cpp
    TranslationClass.h
    UserInterface.cpp
    UserInterface.h
    UserInterface.ui
    anpro/imagecropper.cpp
    anpro/imagecropper.h
    pcg/pcg_basic.c
    pcg/pcg_basic.h
    res/tr_g5p.qrc
    res/app.qrc
    uimod/JSHighlighter.cpp
    uimod/JSHighlighter.h
    uimod/UiModLabel.cpp
    uimod/UiModLabel.h
    uimod/UiModWidget.cpp
    uimod/UiModWidget.h
)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(gta5view PRIVATE config.h)
endif()

target_include_directories(gta5view PRIVATE anpro pcg uimod)
target_link_libraries(gta5view PRIVATE Qt5::Network Qt5::Svg Qt5::Widgets)
target_compile_definitions(gta5view PRIVATE -DGTA5SYNC_PROJECT -DGTA5SYNC_NOASSIST)
