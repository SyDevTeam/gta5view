cmake_minimum_required(VERSION 3.5)

project(gta5view LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Network Svg Widgets REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools QUIET)

set(GTA5VIEW_SOURCES
    main.cpp
    AboutDialog.cpp
    AppEnv.cpp
    CrewDatabase.cpp
    DatabaseThread.cpp
    ExportDialog.cpp
    ExportThread.cpp
    GlobalString.cpp
    IconLoader.cpp
    ImportDialog.cpp
    JsonEditorDialog.cpp
    MapLocationDialog.cpp
    OptionsDialog.cpp
    PictureDialog.cpp
    PictureExport.cpp
    PictureWidget.cpp
    PlayerListDialog.cpp
    ProfileDatabase.cpp
    ProfileInterface.cpp
    ProfileLoader.cpp
    ProfileWidget.cpp
    SavegameCopy.cpp
    SavegameData.cpp
    SavegameDialog.cpp
    SavegameWidget.cpp
    SidebarGenerator.cpp
    SnapmaticEditor.cpp
    SnapmaticPicture.cpp
    SnapmaticWidget.cpp
    StandardPaths.cpp
    StringParser.cpp
    TranslationClass.cpp
    UserInterface.cpp
    anpro/imagecropper.cpp
    pcg/pcg_basic.c
    uimod/JSHighlighter.cpp
    uimod/UiModLabel.cpp
    uimod/UiModWidget.cpp
)

set(GTA5VIEW_HEADERS
    config.h
    AboutDialog.h
    AppEnv.h
    CrewDatabase.h
    DatabaseThread.h
    ExportDialog.h
    ExportThread.h
    GlobalString.h
    IconLoader.h
    ImportDialog.h
    JsonEditorDialog.h
    MapLocationDialog.h
    OptionsDialog.h
    PictureDialog.h
    PictureExport.h
    PictureWidget.h
    PlayerListDialog.h
    ProfileDatabase.h
    ProfileInterface.h
    ProfileLoader.h
    ProfileWidget.h
    SavegameCopy.h
    SavegameData.h
    SavegameDialog.h
    SavegameWidget.h
    SidebarGenerator.h
    SnapmaticEditor.h
    SnapmaticPicture.h
    SnapmaticWidget.h
    StandardPaths.h
    StringParser.h
    TranslationClass.h
    UserInterface.h
    anpro/imagecropper.h
    pcg/pcg_basic.h
    uimod/JSHighlighter.h
    uimod/UiModLabel.h
    uimod/UiModWidget.h
)

set(GTA5VIEW_INCLUDEDIR
    anpro
    pcg
    uimod
)

set(GTA5VIEW_FORMS
    AboutDialog.ui
    ExportDialog.ui
    ImportDialog.ui
    JsonEditorDialog.ui
    MapLocationDialog.ui
    OptionsDialog.ui
    PictureDialog.ui
    PlayerListDialog.ui
    ProfileInterface.ui
    SavegameDialog.ui
    SavegameWidget.ui
    SnapmaticEditor.ui
    SnapmaticWidget.ui
    UserInterface.ui
)

set(GTA5VIEW_DEFINES
    -DGTA5SYNC_CMAKE
    -DGTA5SYNC_PROJECT
    -DGTA5SYNC_NOASSIST
)

set(GTA5VIEW_RESOURCES
    res/app_cmake.qrc
    res/global.qrc
)

set_property(SOURCE res/global.qrc PROPERTY AUTORCC_OPTIONS "-threshold;0;-compress;9")

if(Qt5LinguistTools_FOUND)
    qt5_add_translation(GTA5VIEW_TRANSLATIONS
        res/gta5sync_de.ts
        res/gta5sync_en_US.ts
        res/gta5sync_fr.ts
        res/gta5sync_ko.ts
        res/gta5sync_ru.ts
        res/gta5sync_uk.ts
        res/gta5sync_zh_TW.ts
        res/qtbase_en_GB.ts
    )
    add_custom_target(translations DEPENDS ${GTA5VIEW_TRANSLATIONS})
else()
    set(GTA5VIEW_TRANSLATIONS
        res/gta5sync_de.qm
        res/gta5sync_en_US.qm
        res/gta5sync_fr.qm
        res/gta5sync_ko.qm
        res/gta5sync_ru.qm
        res/gta5sync_uk.qm
        res/gta5sync_zh_TW.qm
        res/qtbase_en_GB.qm
    )
endif()

option(QCONF_BUILD "System installation intended Qconf build" OFF)

if(QCONF_BUILD)
    list(APPEND GTA5VIEW_DEFINES
        -DGTA5SYNC_QCONF
    )
else()
    list(APPEND GTA5VIEW_RESOURCES
        res/tr_g5p.qrc
        res/tr_qt5.qrc
    )
endif()

option(FLATPAK_BUILD "Flatpak modifications and identifications" OFF)

if(FLATPAK_BUILD)
    list(APPEND GTA5VIEW_DEFINES
        "-DGTA5SYNC_BUILDTYPE=\"Flatpak\""
        -DGTA5SYNC_FLATPAK
    )
endif()

add_executable(gta5view
    ${GTA5VIEW_HEADERS}
    ${GTA5VIEW_SOURCES}
    ${GTA5VIEW_FORMS}
    ${GTA5VIEW_RESOURCES}
)

if(Qt5LinguistTools_FOUND AND QCONF_BUILD)
    add_dependencies(gta5view translations)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(gta5view PRIVATE config.h)
endif()

target_compile_definitions(gta5view PRIVATE ${GTA5VIEW_DEFINES})
target_include_directories(gta5view PRIVATE ${GTA5VIEW_INCLUDEDIR})
target_link_libraries(gta5view PRIVATE Qt5::Network Qt5::Svg Qt5::Widgets)

install(TARGETS gta5view DESTINATION bin)
install(FILES res/de.syping.gta5view.desktop DESTINATION share/applications)
install(FILES res/gta5view-16.png DESTINATION share/icons/hicolor/16x16/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-24.png DESTINATION share/icons/hicolor/24x24/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-32.png DESTINATION share/icons/hicolor/32x32/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-48.png DESTINATION share/icons/hicolor/48x48/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-64.png DESTINATION share/icons/hicolor/64x64/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-96.png DESTINATION share/icons/hicolor/96x96/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-128.png DESTINATION share/icons/hicolor/128x128/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-256.png DESTINATION share/icons/hicolor/256x256/apps RENAME de.syping.gta5view.png)
install(FILES res/gta5view-512.png DESTINATION share/icons/hicolor/512x512/apps RENAME de.syping.gta5view.png)
install(FILES res/de.syping.gta5view.png DESTINATION share/pixmaps)

if(QCONF_BUILD)
    install(FILES ${GTA5VIEW_TRANSLATIONS} DESTINATION share/gta5view/translations)
endif()
